#include "heart/io/io_cmd_queue.h"
#include "heart/io/io_cmd_list.h"
#include "heart/io/io_op_type.h"

#include "heart/countof.h"
#include "heart/file.h"
#include "heart/stream.h"

#include "heart/sync/fence.h"

#include <algorithm>

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

IoCmdQueue::IoCmdQueue() :
	m_thread(&ThreadEntryPoint, this)
{
	m_thread.SetName("IoCmdQueue Thread");
}

IoCmdQueue::~IoCmdQueue()
{
	Close();
}

void IoCmdQueue::Flush()
{
	HeartUniqueLock lock(m_mutex);

	while (std::any_of(std::begin(m_pages), std::end(m_pages), [](const CmdPage& page) { return page.inUse; }))
	{
		lock.Unlock();
		Sleep(0);
		lock.Lock();
	}
}

void IoCmdQueue::Close()
{
	if (m_thread)
	{
		Flush();

		m_threadExit = true;
		m_thread.Join();
	}
}

void IoCmdQueue::Submit(IoCmdList* cmdList)
{
	HeartLockGuard lock(m_mutex);

	// Find a free page
	CmdPage* tgtPage = nullptr;
	for (auto& p : m_pages)
	{
		if (!p.inUse)
		{
			tgtPage = &p;
		}
	}

	// Copy our data
	tgtPage->size = cmdList->m_writeHead;
	memcpy_s(tgtPage->data, HeartCountOf(tgtPage->data), cmdList->m_cmdPool, cmdList->m_writeHead);

	// Point to the new page
	CmdPage** tgt = &m_head;
	while (*tgt != nullptr)
	{
		tgt = &(*tgt)->next;
	}

	*tgt = tgtPage;

	// Reset the cmd list
	cmdList->Finalize();

	// Wake the thread
	m_cv.NotifyOne();
}

void IoCmdQueue::ThreadThink()
{
	CmdPage* workingPage = nullptr;

	while (m_threadExit.load(std::memory_order_relaxed) == false)
	{
		{
			HeartLockGuard lock(m_mutex);

			if (workingPage != nullptr)
			{
				workingPage->inUse = false;
				workingPage = nullptr;
			}

			while (m_head == nullptr)
			{
				m_cv.Wait(m_mutex);
			}

			workingPage = m_head;
			m_head = workingPage->next;
		}

		ProcessCmdPage(*workingPage);
	}
}

void IoCmdQueue::ProcessCmdPage(CmdPage& page)
{
	uint16_t readHead = 0;

	struct IoState
	{
		// TODO: true async io
		HeartFile currentFile = {};
		const IoFileDescriptor* currentDescriptor = nullptr;
		void* currentTargetBuffer = nullptr;
		int64_t currentTargetBufferSize = -1;
	} state;

	HeartStreamReader reader(page.data, &readHead);
	while (readHead < page.size)
	{
		IoOpType operation = reader.Read<IoOpType>(reader.Copy);

		switch (operation)
		{
		case IoOpType::BindDescriptor: {
			const IoFileDescriptor* d = reader.Read<IoFileDescriptor>(reader.GetPtr);
			state.currentDescriptor = d;

			if (state.currentFile)
				HeartCloseFile(state.currentFile);

			HeartOpenFile(state.currentFile, state.currentDescriptor->GetFilename(), HeartOpenFileMode::ReadExisting);

			break;
		}
		case IoOpType::BindBufferUnchecked: {
			IoUncheckedTargetBuffer buffer = reader.Read<IoUncheckedTargetBuffer>(reader.Copy);
			state.currentTargetBuffer = buffer.ptr;
			state.currentTargetBufferSize = -1;
			break;
		}
		case IoOpType::BindBufferChecked: {
			IoCheckedTargetBuffer buffer = reader.Read<IoCheckedTargetBuffer>(reader.Copy);
			state.currentTargetBuffer = buffer.ptr;
			state.currentTargetBufferSize = int64_t(buffer.size);
			break;
		}
		case IoOpType::ReadEntire: {
			HEART_ASSERT(state.currentDescriptor != nullptr);
			HEART_ASSERT(state.currentTargetBuffer != nullptr);

			if (state.currentFile)
			{
				uint64_t size = 0;
				if (HeartGetFileSize(state.currentFile, size))
				{
					size_t bufferSize = state.currentTargetBufferSize < 0 ? size_t(size) : size_t(state.currentTargetBufferSize);
					if (state.currentTargetBufferSize < 0 || int64_t(size) <= state.currentTargetBufferSize)
					{
						HeartReadFile(state.currentFile, (byte_t*)state.currentTargetBuffer, bufferSize, size);
					}
				}
			}

			break;
		}
		case IoOpType::ReadPartial: {
			HEART_ASSERT(state.currentDescriptor != nullptr);
			HEART_ASSERT(state.currentTargetBuffer != nullptr);

			size_t toRead = reader.Read<size_t>(reader.Copy);

			if (state.currentFile)
			{
				size_t bufferSize = state.currentTargetBufferSize < 0 ? toRead : size_t(state.currentTargetBufferSize);
				HeartReadFile(state.currentFile, (byte_t*)state.currentTargetBuffer, bufferSize, toRead);
			}

			break;
		}
		case IoOpType::Offset: {
			HEART_ASSERT(state.currentDescriptor != nullptr);

			int64_t offset = reader.Read<int64_t>(reader.Copy);
			IoOffsetType type = reader.Read<IoOffsetType>(reader.Copy);

			if (state.currentFile)
			{
				HeartSetOffsetMode heartMode = HeartSetOffsetMode::Beginning;
				switch (type)
				{
				case IoOffsetType::FromStart: heartMode = HeartSetOffsetMode::Beginning; break;
				case IoOffsetType::FromCurrent: heartMode = HeartSetOffsetMode::Current; break;
				case IoOffsetType::FromEnd: heartMode = HeartSetOffsetMode::End; break;
				}

				HeartSetFileOffset(state.currentFile, offset, nullptr, heartMode);
			}

			break;
		}
		case IoOpType::UnbindDescriptor: {
			state.currentDescriptor = nullptr;
			break;
		}
		case IoOpType::UnbindTarget: {
			state.currentTargetBuffer = nullptr;
			state.currentTargetBufferSize = -1;
			break;
		}
		case IoOpType::Reset: {
			break;
		}
		case IoOpType::SignalFence: {
			HeartFence* fence = reader.Read<HeartFence*>(reader.Copy);
			uint32_t value = reader.Read<uint32_t>(reader.Copy);
			fence->Signal(value);
			break;
		}
		case IoOpType::WaitForFence: {
			HeartFence* fence = reader.Read<HeartFence*>(reader.Copy);
			uint32_t value = reader.Read<uint32_t>(reader.Copy);
			fence->Wait(value);
			break;
		}
		}
	}
}

void* IoCmdQueue::ThreadEntryPoint(void* p)
{
	IoCmdQueue* q = (IoCmdQueue*)p;
	q->ThreadThink();
	return nullptr;
}

#include "heart/io/io_cmd_queue.h"
#include "heart/io/io_cmd_list.h"
#include "heart/io/io_op_type.h"

#include "heart/countof.h"
#include "heart/file.h"
#include "heart/stream.h"

#include "heart/sync/fence.h"

#include <algorithm>
#include <iterator>

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

IoCmdQueue::IoCmdQueue(int threadCount)
{
	threadCount = std::max(threadCount, 1);
	std::generate_n(std::back_inserter(m_threads), threadCount, [this]() {
		HeartThread t(&ThreadEntryPoint, this);
		t.SetName("IoCmdQueue Thread");
		return hrt::move(t);
	});
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
	if (!m_threads.empty())
	{
		Flush();

		m_threadExit = true;
		m_cv.NotifyAll();

		for (auto& t : m_threads)
			t.Join();

		m_threads.clear();
	}
}

void IoCmdQueue::Submit(IoCmdList* cmdList)
{
	{
		HeartUniqueLock lock(m_mutex);

		// Find a free page
		CmdPage* tgtPage = nullptr;
		for (auto& p : m_pages)
		{
			if (!p.inUse)
			{
				tgtPage = &p;
				break;
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
	}

	// Reset the cmd list
	cmdList->Finalize();

	// Wake the thread
	m_cv.NotifyAll();
}

void IoCmdQueue::ThreadThink()
{
	CmdPage* workingPage = nullptr;

	while (true)
	{
		{
			HeartUniqueLock lock(m_mutex);

			if (workingPage != nullptr)
			{
				workingPage->inUse = false;
				workingPage = nullptr;
			}

			while (m_head == nullptr && m_threadExit.load(std::memory_order_relaxed) == false)
			{
				m_cv.Wait(m_mutex);
			}

			if (m_threadExit)
				break;

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
		bool descriptorBound = false;
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
			uint8_t pathSize = reader.Read<uint8_t>(reader.Copy);
			const char* pathStr = reader.ReadSpan<char>(pathSize);

			state.descriptorBound = true;

			if (state.currentFile)
				HeartCloseFile(state.currentFile);

			char path[MAX_PATH] = {};
			strncpy_s(path, pathStr,  pathSize);

			HeartOpenFile(state.currentFile, path, HeartOpenFileMode::ReadExisting);

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
			HEART_ASSERT(state.descriptorBound);
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
			HEART_ASSERT(state.descriptorBound);
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
			HEART_ASSERT(state.descriptorBound);

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
			state.descriptorBound = false;
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

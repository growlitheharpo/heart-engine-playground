#include <heart/thread.h>

#include <heart/debug/assert.h>

#include <atomic>

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

struct Passthrough
{
	HeartThread::EntryPoint f = nullptr;
	void* u = nullptr;
};

// TODO: this is not exactly safe
static Passthrough s_passthroughPool[256] = {};
static std::atomic<uint8_t> s_passthroughIndex;

static DWORD WINAPI GlobalThreadEntryPoint(LPVOID param)
{
	uint8_t index = (uint8_t)(uintptr_t)param;
	Passthrough& pass = s_passthroughPool[index];

	void* r = pass.f(pass.u);

	return r == nullptr ? 0 : 1;
}

const int PriorityRemap[] = {
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_TIME_CRITICAL,
};

HeartThread::HeartThread(EntryPoint entryPoint, void* userData, Priority priority, uint32_t stackSize)
{
	uint8_t index = s_passthroughIndex.fetch_add(1);
	Passthrough& pass = s_passthroughPool[index];
	pass.f = entryPoint;
	pass.u = userData;

	m_handle = ::CreateThread(
		nullptr,
		stackSize,
		&GlobalThreadEntryPoint,
		(void*)index,
		0,
		nullptr);

	if (m_handle)
	{
		SetThreadPriority(m_handle, PriorityRemap[(int)priority]);
	}
}

HeartThread::~HeartThread()
{
	HEART_ASSERT(!m_handle, "Thread is being destroyed without being explicitly joined or detached!");
	if (m_handle)
	{
		Detach();
	}
}

HeartThread::HeartThread(HeartThread&& o) noexcept
{
	this->m_handle = o.m_handle;
	o.m_handle = nullptr;
}

HeartThread& HeartThread::operator=(HeartThread&& o) noexcept
{
	this->m_handle = o.m_handle;
	o.m_handle = nullptr;
	return *this;
}

void HeartThread::Join()
{
	HANDLE h = m_handle;
	m_handle = nullptr;

	WaitForSingleObject(h, INFINITE);
}

void HeartThread::Detach()
{
	HANDLE h = m_handle;
	m_handle = nullptr;

	CloseHandle(h);
}

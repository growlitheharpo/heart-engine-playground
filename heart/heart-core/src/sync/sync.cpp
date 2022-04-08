#include "heart/sync/condition_variable.h"
#include "heart/sync/event.h"
#include "heart/sync/fence.h"
#include "heart/sync/mutex.h"

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2012/november/windows-with-c-the-evolution-of-synchronization-in-windows-and-c#slim-readerwriter-lock
#include "priv/SlimWin32.h"

HeartConditionVariable::HeartConditionVariable()
{
	static_assert(sizeof(m_handle) <= sizeof(CONDITION_VARIABLE), "CONDITION_VARIABLE has grown! We need more storage.");
	m_handle = nullptr;

	CONDITION_VARIABLE& cv = *(CONDITION_VARIABLE*)NativeHandle();
	InitializeConditionVariable(&cv);
}

HeartConditionVariable::~HeartConditionVariable()
{
	m_handle = nullptr;
}

void HeartConditionVariable::NotifyOne()
{
	CONDITION_VARIABLE& cv = *(CONDITION_VARIABLE*)NativeHandle();
	WakeConditionVariable(&cv);
}

void HeartConditionVariable::NotifyAll()
{
	CONDITION_VARIABLE& cv = *(CONDITION_VARIABLE*)NativeHandle();
	WakeAllConditionVariable(&cv);
}

void HeartConditionVariable::Wait(HeartMutex& mutex, WaitOwnership ownership)
{
	SRWLOCK& lock = *(SRWLOCK*)mutex.NativeHandle();
	CONDITION_VARIABLE& cv = *(CONDITION_VARIABLE*)NativeHandle();

	ULONG flags = ownership == WaitOwnership::Shared ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0;
	SleepConditionVariableSRW(&cv, &lock, INFINITE, flags);
}

bool HeartConditionVariable::TryWaitFor(HeartMutex& mutex, uint32_t milliseconds, WaitOwnership ownership)
{
	SRWLOCK& lock = *(SRWLOCK*)mutex.NativeHandle();
	CONDITION_VARIABLE& cv = *(CONDITION_VARIABLE*)NativeHandle();

	ULONG flags = ownership == WaitOwnership::Shared ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0;
	return SleepConditionVariableSRW(&cv, &lock, milliseconds, flags);
}

HeartMutex::HeartMutex()
{
	static_assert(sizeof(m_handle) <= sizeof(SRWLOCK), "SRWLOCK has grown! We need more storage.");
	m_handle = nullptr;

	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();
	InitializeSRWLock(&lock);
}

HeartMutex::~HeartMutex()
{
	// An unlocked SRW lock with no waiting threads is in its initial state and can be copied, moved, and forgotten without being explicitly destroyed.
}

void HeartMutex::LockExclusive()
{
	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();
	AcquireSRWLockExclusive(&lock);
}

bool HeartMutex::TryLockExclusive()
{
	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();
	return TryAcquireSRWLockExclusive(&lock) != FALSE;
}

void HeartMutex::LockShared()
{
	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();
	AcquireSRWLockShared(&lock);
}

bool HeartMutex::TryLockShared()
{
	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();
	return TryAcquireSRWLockShared(&lock) != FALSE;
}

void HeartMutex::Unlock()
{
	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();

// Suppress warning about releasing a lock we don't own.
#pragma warning(suppress : 26110)
	ReleaseSRWLockExclusive(&lock);
}

void HeartMutex::UnlockShared()
{
	SRWLOCK& lock = *(SRWLOCK*)NativeHandle();
	ReleaseSRWLockShared(&lock);
}

void HeartFence::Signal(uint32_t revision)
{
	{
		HeartLockGuard lock(m_mutex);
		m_currentRevision = revision;
	}

	m_cv.NotifyAll();
}

void HeartFence::Wait(uint32_t revision)
{
	HeartLockGuard lock(m_mutex);

	while (m_currentRevision < revision)
	{
		m_cv.Wait(m_mutex);
	}
}

bool HeartFence::Test(uint32_t revision)
{
	HeartLockGuard lock(m_mutex);
	return m_currentRevision >= revision;
}

HeartEvent::HeartEvent(ResetType rt)
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	handle = ::CreateEvent(NULL, rt == ResetType::Manual ? TRUE : FALSE, FALSE, NULL);
}

HeartEvent::~HeartEvent()
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	CloseHandle(handle);
	handle = NULL;
}

void HeartEvent::Set()
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	::SetEvent(handle);
}

void HeartEvent::Reset()
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	::ResetEvent(handle);
}

void HeartEvent::Wait()
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	::WaitForSingleObject(handle, INFINITE);
}

void HeartEvent::Wait(uint32_t waitDurationMs)
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	::WaitForSingleObject(handle, DWORD(waitDurationMs));
}

void HeartEvent::SignalAndWait(HeartEvent& other)
{
	HANDLE& handle = GetNativeHandleAs<HANDLE>();
	::SetEvent(handle);
	::WaitForSingleObject(other.GetNativeHandleAs<HANDLE>(), INFINITE);
}

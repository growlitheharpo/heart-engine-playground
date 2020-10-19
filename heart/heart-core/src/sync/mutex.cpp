#include <heart/sync/mutex.h>

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2012/november/windows-with-c-the-evolution-of-synchronization-in-windows-and-c#slim-readerwriter-lock
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

HeartMutex::HeartMutex()
{
	static_assert(sizeof(m_handle) == sizeof(SRWLOCK), "SRWLOCK has grown! We need more storage.");
	m_handle = nullptr;

	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	InitializeSRWLock(&lock);
}

HeartMutex::~HeartMutex()
{
	// An unlocked SRW lock with no waiting threads is in its initial state and can be copied, moved, and forgotten without being explicitly destroyed.
}

void HeartMutex::LockExclusive()
{
	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	AcquireSRWLockExclusive(&lock);
}

bool HeartMutex::TryLockExclusive()
{
	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	return TryAcquireSRWLockExclusive(&lock) != FALSE;
}

void HeartMutex::LockShared()
{
	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	AcquireSRWLockShared(&lock);
}

bool HeartMutex::TryLockShared()
{
	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	return TryAcquireSRWLockShared(&lock) != FALSE;
}

void HeartMutex::Unlock()
{
	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	ReleaseSRWLockExclusive(&lock);
}

void HeartMutex::UnlockShared()
{
	SRWLOCK& lock = *(SRWLOCK*)&m_handle;
	ReleaseSRWLockShared(&lock);
}

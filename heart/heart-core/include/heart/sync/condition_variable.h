#pragma once

#include <heart/copy_move_semantics.h>
#include <heart/types.h>

class HeartMutex;

class HeartConditionVariable
{
public:
	enum class WaitOwnership
	{
		Shared,
		Exclusive
	};

private:
	void* m_handle;

public:
	HeartConditionVariable();
	~HeartConditionVariable();

	DISABLE_COPY_AND_MOVE_SEMANTICS(HeartConditionVariable);

	void NotifyOne();
	void NotifyAll();
	void Wait(HeartMutex& mutex, WaitOwnership = WaitOwnership::Exclusive);
	bool TryWaitFor(HeartMutex& mutex, uint32_t milliseconds, WaitOwnership = WaitOwnership::Exclusive);

	void** NativeHandle()
	{
		return &m_handle;
	}
};

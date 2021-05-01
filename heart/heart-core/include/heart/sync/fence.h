#pragma once

#include <heart/sync/condition_variable.h>
#include <heart/sync/mutex.h>

class HeartFence
{
private:
	HeartConditionVariable m_cv;
	HeartMutex m_mutex;
	uint32_t m_currentRevision = 0;

public:
	HeartFence() = default;
	~HeartFence() = default;

	DISABLE_COPY_AND_MOVE_SEMANTICS(HeartFence);

	void Signal(uint32_t revision);
	void Wait(uint32_t revision);

	bool Test(uint32_t revision);
};

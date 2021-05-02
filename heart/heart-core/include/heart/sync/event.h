#pragma once

#include <heart/types.h>

#include <heart/copy_move_semantics.h>

// TODO!
class HeartEvent
{
public:
	enum class ResetType
	{
		Manual,
		Automatic,
	};

public:
	HeartEvent(ResetType rt = ResetType::Automatic);
	~HeartEvent();

	DISABLE_COPY_AND_MOVE_SEMANTICS(HeartEvent);

	void Set();
	void Reset();
	void Wait();
	void Wait(uint32_t waitDurationMs);

	void SignalAndWait(HeartEvent& other);
};

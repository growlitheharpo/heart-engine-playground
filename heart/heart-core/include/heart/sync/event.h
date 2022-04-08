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

private:
	uintptr_t m_handle = 0;

	uintptr_t* NativeHandle()
	{
		return &m_handle;
	}

	template <typename T>
	T& GetNativeHandleAs()
	{
		return *(T*)NativeHandle();
	}

public:
	HeartEvent(ResetType rt = ResetType::Automatic);
	~HeartEvent();

	DISABLE_COPY_SEMANTICS(HeartEvent);

	USE_DEFAULT_MOVE_SEMANTICS(HeartEvent);

	void Set();
	void Reset();
	void Wait();
	void Wait(uint32_t waitDurationMs);

	void SignalAndWait(HeartEvent& other);
};

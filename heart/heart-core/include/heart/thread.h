#pragma once

#include <heart/types.h>

#include <heart/copy_move_semantics.h>

class HeartThread
{
private:
	void* m_handle = nullptr;

public:
	enum class Priority
	{
		Lowest,
		Low,
		Normal,
		High,
		Highest,
		Critical,

		Count
	};

	typedef void* (*EntryPoint)(void*);

	HeartThread(EntryPoint entryPoint, void* userData = nullptr, Priority priority = Priority::Normal, uint32_t stackSize = 0);
	~HeartThread();

	DISABLE_COPY_SEMANTICS(HeartThread);

	HeartThread(HeartThread&& o) noexcept;
	HeartThread& operator=(HeartThread&& o) noexcept;

	void Join();
	void Detach();
};

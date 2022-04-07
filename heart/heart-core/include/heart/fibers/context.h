#pragma once

#include <heart/fibers/fwd.h>

#include <heart/fibers/work_unit.h>

class HeartFiberContext
{
private:
	friend class HeartFiberSystem;

	// The system that is executing the current thread. If we're on
	// a fiber, should *never* be null.
	HeartFiberSystem* currentSystem;

	// The pump for the current thread. If we're on a fiber, should always be valid
	// until the thread exits. The pump executes HeartFiberSystem::PumpRoutine
	HeartFiberWorkUnit pump;

	// The currently executing work unit. If we're on a fiber, should
	// never be null after the first switch to the pump
	HeartFiberWorkUnit* currentWorkUnit;

	// The destroy queue for this thread. Only work units which have completed
	// on this thread can be destroyed by this thread. This prevents a race
	// condition where the pump running on one thread could kill a currently
	// executing fiber on another thread before it jumps, causing a crash.
	HeartFiberWorkUnit::Queue destroyQueue;

	static HeartFiberContext& Get();

public:
	// Attempt to yield the current fiber. This will return immediately
	// if we are not currently executing within a fiber, and may effectively
	// return immediately if there is no other queued work.
	static void Yield();
};

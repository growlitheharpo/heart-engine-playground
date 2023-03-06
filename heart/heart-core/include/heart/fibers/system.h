/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <heart/fibers/fwd.h>
#include <heart/fibers/work_unit.h>

#include <heart/allocator.h>
#include <heart/memory/intrusive_list.h>
#include <heart/memory/intrusive_ptr.h>
#include <heart/memory/vector.h>
#include <heart/sync/mutex.h>
#include <heart/thread/thread.h>

#include <heart/sleep.h>

#include <atomic>

class HeartFiberSystem
{
public:
	struct Settings
	{
		uint32_t threadCount = 1;
	};

private:
	friend class HeartFiberContext;

	// Allocator for the system. All allocations must be pulled from this and
	// not from any global allocation source. Any failure to allocate will
	// be instantly fatal and is not handled - the owner of this HeartFiberSystem
	// MUST ensure the allocator is always successful.
	HeartBaseAllocator& m_allocator;

	// The list of threads used by this system for fibers
	heart_priv::HeartVector<HeartThread> m_threads;

	// List of fiber work awaiting execution. Mutex protected because
	// the main thread can push work into it and the fiber thread(s)
	// needs to both push and pull.
	HeartFiberWorkUnit::Queue m_pendingQueue;
	HeartMutex m_pendingQueueMutex;

	// Flag to signal to the pumps that the system is shutting down.
	std::atomic_bool m_exit = true;

private:
	// Static entry pointer for threads which will become fibers.
	void HeartFiberThreadEntry();

	// Global entry pointer for fiber work units. Invokes the work unit's
	// worker, which can yield as many times as it likes. When it eventually
	// exits, passes execution of the fiber back to the pump.
	static void HeartFiberStartRoutine(void*);

	// The pump routine. Keeps all fiber threads alive throughout the
	// system's lifetime and is responsible for pulling work units for each
	// thread from the queue
	HeartFiberResult PumpRoutine();

	// Sets up the entry fiber, if necessary. Called from thread entry and nowhere else.
	void InitializeEntryFiber();

	// Releases the entry fiber, if necessary. Called from thread entry and nowhere else.
	void ReleaseEntryFiber();

	// Prepare a work unit for actual execution by creating a native fiber for it.
	// This can only be called from a thread which is already a fiber, so it is
	// delegated to the pump for user work.
	void InitializeWorkUnitNativeHandle(HeartFiberWorkUnit& unit);

	// Release any resources allocated by the native fiber for this work unit.
	// This must be called after the fiber has finished executing on all threads.
	// Calling this e.g while the fiber is still in the process of yielding will crash.
	void ReleaseWorkUnitNativeHandle(HeartFiberWorkUnit& unit);

	// Finalize a work unit. Adds it to the destroy queue and passes execution
	// to the pump.
	void CompleteWorkUnit(HeartFiberWorkUnit& unit);

	// Requeue a work unit. Internally, creates a copy with the same worker
	// and adds the old work unit to the destroy queue before passing execution
	// to the pump.
	void RequeueWorkUnit(HeartFiberWorkUnit& unit);

	// Yield the currently executing unit. Passes execution to the pump.
	void YieldUnit(HeartFiberWorkUnit& unit);

	// Invoke the native method to pass execution to the given work unit.
	// Could return, if the current fiber is switched back to
	void NativeSwitchToFiber(HeartFiberWorkUnit& target);

	// Invoke the native method to pass execution to the given work unit.
	// Can only be used in contexts where we know the current fiber will
	// never be switched back to (eg the startup context, and when a work
	// unit completes).
	[[noreturn]] void NativeSwitchToFiberNoReturn(HeartFiberWorkUnit& target);

public:
	HeartFiberSystem(HeartBaseAllocator& allocator = GetHeartDefaultAllocator());
	~HeartFiberSystem();

	void Initialize(Settings s);
	void Shutdown();

	// Add a work unit to the fiber system. Will be executed at some later point.
	// F must be movable but is not required to be copyable.
	template <typename F>
	HeartFiberWorkUnitRef EnqueueWork(F&& f)
	{
		HeartFiberWorkUnit* newWorkUnit = m_allocator.AllocateAndConstruct<HeartFiberWorkUnit>(
			HeartFiberWorkUnit::ConstructorSecret,
			&m_allocator,
			hrt::forward<F>(f));

		{
			HeartLockGuard lock(m_pendingQueueMutex);

			// We manually increment the ref count here. This is the "queue ref"
			// since the queue itself holds raw pointers, not intrusive pointers
			newWorkUnit->IncrementRef();
			m_pendingQueue.PushBack(newWorkUnit);
		}

		return HeartFiberWorkUnitRef(newWorkUnit);
	}
};

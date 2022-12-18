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
#include "heart/fibers/system.h"

#include "heart/fibers/context.h"
#include "heart/fibers/status.h"
#include "heart/fibers/work_unit.h"

#include "heart/debug/assert.h"

#include "fibers/native_impl.h"

#include <stdio.h>

struct HeartFiberThreadEntryParam
{
	enum State
	{
		Setup,
		Ready,
		Done,
	};

	std::atomic<State> state = State::Setup;

	HeartFiberSystem* system = nullptr;
};

void* HeartFiberSystem::HeartFiberThreadEntry(void* parameter)
{
	// Wait for the main thread to fully populate the parameter
	auto currentThread = (HeartFiberThreadEntryParam*)parameter;
	while (currentThread->state < HeartFiberThreadEntryParam::Ready)
	{
		HeartYield();
	}

	// Prepare the context for this thread
	HeartFiberContext::Get().currentSystem = currentThread->system;
	HeartFiberContext::Get().pump.m_worker = []() { return HeartFiberContext::Get().currentSystem->PumpRoutine(); };

	// Tell the main thread we're done reading the parameter
	currentThread->state = HeartFiberThreadEntryParam::Done;
	currentThread = nullptr;

	// Allocate a startup abi so that the system has something to jump away from
	HeartFiberContext::Get().currentSystem->InitializeEntryFiber();
	HeartFiberContext::Get().currentWorkUnit = &HeartFiberContext::Get().entryUnit;

	// Initialize and execute the pump
	HeartFiberContext::Get().currentSystem->InitializeWorkUnitNativeHandle(HeartFiberContext::Get().pump);
	HeartFiberContext::Get().currentSystem->NativeSwitchToFiber(HeartFiberContext::Get().pump);

	// We've returned from the pump, time to cleanup and kill the thread.
	HEART_ASSERT(HeartFiberContext::Get().currentSystem->m_exit == true);
	HeartFiberContext::Get().currentSystem->ReleaseWorkUnitNativeHandle(HeartFiberContext::Get().pump);

	// Release the entry fiber - actual behavior is implementation-dependent (since we're currently *on* it).
	HeartFiberContext::Get().currentSystem->ReleaseEntryFiber();

	return nullptr;
}

void HeartFiberSystem::HeartFiberStartRoutine(void*)
{
	// Verify that the fiber native code gave us a valid stack
	HEART_FIBER_VERIFY_STACK();

	// Get the work unit and system from the context
	HeartFiberWorkUnit& workUnit = *HeartFiberContext::Get().currentWorkUnit;
	HeartFiberSystem* system = HeartFiberContext::Get().currentSystem;

	// Begin execution of the work unit.
	HeartFiberStatus result = workUnit.m_worker();

	if (&workUnit == &HeartFiberContext::Get().pump)
	{
		// If the work unit was the pump, we're done. This will kill the thread.
		system->NativeSwitchToFiber(HeartFiberContext::Get().entryUnit);
	}
	else if (result == HeartFiberStatus::Complete)
	{
		// Otherwise, if it completed, tell the system it's done.
		// The system will handle switching to the pump.
		system->CompleteWorkUnit(workUnit);
	}
	else
	{
		// Otherwise, if it needs to be requeued, inform the system.
		// The system will handle switching to the pump.
		system->RequeueWorkUnit(workUnit);
	}
}

HeartFiberStatus HeartFiberSystem::PumpRoutine()
{
	auto canExit = [&]() {
		if (!m_exit)
			return false;

		bool destroyEmpty = HeartFiberContext::Get().destroyQueue.IsEmpty();
		if (!destroyEmpty)
			return false;

		HeartLockGuard lock(m_pendingQueueMutex);
		return m_pendingQueue.IsEmpty();
	};

	while (!canExit())
	{
		// Destroy any fibers that completed on this thread.
		while (!HeartFiberContext::Get().destroyQueue.IsEmpty())
		{
			HeartFiberWorkUnit* unit = HeartFiberContext::Get().destroyQueue.PopFront();
			ReleaseWorkUnitNativeHandle(*unit);
			m_allocator.DestroyAndFree(unit);
		}

		// Grab our next work unit
		HeartFiberWorkUnit* next = nullptr;
		{
			HeartLockGuard lock(m_pendingQueueMutex);
			next = m_pendingQueue.PopFront();
		}

		if (next != nullptr)
		{
			if (next->m_nativeHandle == nullptr)
			{
				InitializeWorkUnitNativeHandle(*next);
			}

			NativeSwitchToFiber(*next);
			next = nullptr;
		}
		else
		{
			HeartSleep(5);
		}
	}

	return HeartFiberStatus::Complete;
}

void HeartFiberSystem::CompleteWorkUnit(HeartFiberWorkUnit& unit)
{
	HeartFiberContext::Get().destroyQueue.PushBack(&unit);

	NativeSwitchToFiberNoReturn(HeartFiberContext::Get().pump);
}

void HeartFiberSystem::RequeueWorkUnit(HeartFiberWorkUnit& unit)
{
	EnqueueFiber(hrt::move(unit.m_worker));

	CompleteWorkUnit(unit);
}

void HeartFiberSystem::YieldUnit(HeartFiberWorkUnit& unit)
{
	// Verify upon leaving that we have a valid stack
	HEART_FIBER_VERIFY_STACK();

	{
		HeartLockGuard lock(m_pendingQueueMutex);
		m_pendingQueue.PushBack(&unit);
	}

	NativeSwitchToFiber(HeartFiberContext::Get().pump);

	// And verify upon coming back that we have a valid stack
	HEART_FIBER_VERIFY_STACK();
}

void HeartFiberSystem::NativeSwitchToFiberNoReturn(HeartFiberWorkUnit& target)
{
	// Switch to the fiber. It should never return to us.
	NativeSwitchToFiber(target);

	// Uh oh. the fiber came back. Kill the thread with an error code.
	// It should've swapped back to the pump or the entry fiber!
	HEART_ASSERT(false, "Error: fiber returned when it should not have!");
	HeartExitThread(-1);
}

HeartFiberSystem::HeartFiberSystem(HeartBaseAllocator& allocator) :
	m_allocator(allocator),
	m_threads(allocator)
{
}

HeartFiberSystem::~HeartFiberSystem()
{
	HEART_ASSERT(m_exit);
	HEART_ASSERT(m_threads.IsEmpty());
}

void HeartFiberSystem::Initialize(Settings s)
{
	m_exit = false;

	m_threads.Reserve(s.threadCount);
	for (uint32_t i = 0; i < s.threadCount; ++i)
	{
		HeartFiberThreadEntryParam param;
		param.system = this;

		HeartThread& thread = m_threads.EmplaceBack(HeartThread(&HeartFiberThreadEntry, &param));
		param.state = HeartFiberThreadEntryParam::Ready;

		char buffer[64];
		sprintf_s(buffer, "HeartFiber Thread %u", i + 1);
		thread.SetName(buffer);

		while (param.state < HeartFiberThreadEntryParam::Done)
		{
			HeartYield();
		}
	}
}

void HeartFiberSystem::Shutdown()
{
	m_exit = true;

	for (auto& thread : m_threads)
		thread.Join();

	m_threads.Clear();
}

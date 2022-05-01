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

#include "fibers/abi_context.h"

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

	// Put the startup context into the destroy queue
	auto* startupAbi = HeartFiberContext::Get().currentSystem->m_allocator.AllocateAndConstruct<HeartFiberAbiContext>();
	*startupAbi = {};
	HeartFiberContext::Get().entryUnit.m_nativeHandle = startupAbi;
	HeartFiberContext::Get().currentWorkUnit = &HeartFiberContext::Get().entryUnit;
	startupAbi = nullptr;

	// Initialize and execute the pump
	HeartFiberContext::Get().currentSystem->InitializeWorkUnitNativeHandle(HeartFiberContext::Get().pump);
	HeartFiberContext::Get().currentSystem->NativeSwitchToFiber(HeartFiberContext::Get().pump);

	// We've returned from the pump, time to kill the thread.
	HEART_ASSERT(HeartFiberContext::Get().currentSystem->m_exit == true);
	HeartFiberContext::Get().currentSystem->ReleaseWorkUnitNativeHandle(HeartFiberContext::Get().pump);
	HeartFiberContext::Get().currentSystem->ReleaseWorkUnitNativeHandle(HeartFiberContext::Get().entryUnit);

	return nullptr;
}

void HeartFiberSystem::HeartFiberStartRoutine()
{
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
		bool destroyEmpty = HeartFiberContext::Get().destroyQueue.IsEmpty();

		bool pendingEmpty;
		{
			HeartLockGuard lock(m_pendingQueueMutex, HeartFiberMutex::NeverYield {});
			pendingEmpty = m_pendingQueue.IsEmpty();
		}

		return destroyEmpty && pendingEmpty && m_exit;
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
			HeartLockGuard lock(m_pendingQueueMutex, HeartFiberMutex::NeverYield {});
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

void HeartFiberSystem::InitializeWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	// We can only create fibers from a fiber!
	HEART_ASSERT(HeartFiberContext::Get().currentSystem != nullptr);

	// unit.m_nativeHandle = ::CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, &HeartFiberStartRoutine, NULL);
	constexpr uint32_t StackSize = 16u * 1024u;

	HeartFiberAbiContext* ctx = m_allocator.AllocateAndConstruct<HeartFiberAbiContext>();
	byte_t* stack = m_allocator.allocate<byte_t>(StackSize);
	memset(ctx, 0, sizeof(HeartFiberAbiContext));
	memset(stack, 0, StackSize);
	ctx->allocated = stack;

	// Stacks grow downward, point to the end of our buffer
	stack = stack + StackSize;
	// Win64 requires 16bit alignment of the stack
	stack = (byte_t*)(uintptr_t(stack) & -16L);
	// Win64 requires space for 4 64bit values on top of any actual arguments
	stack -= 4 * sizeof(uint64_t);

	ctx->rsp = uintptr_t(stack);
	ctx->rbp = uintptr_t(stack);
	ctx->rip = uintptr_t(&HeartFiberStartRoutine);

	unit.m_nativeHandle = ctx;
}

void HeartFiberSystem::ReleaseWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	HEART_ASSERT(HeartFiberContext::Get().currentSystem != nullptr);

	HeartFiberAbiContext* ctx = reinterpret_cast<HeartFiberAbiContext*>(unit.m_nativeHandle);
	unit.m_nativeHandle = nullptr;

	// Free the stack
	m_allocator.deallocate(ctx->allocated);
	ctx->allocated = nullptr;

	// Free the actual abi context
	m_allocator.DestroyAndFree<HeartFiberAbiContext>(ctx);
	ctx = nullptr;
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
	{
		HeartLockGuard lock(m_pendingQueueMutex, HeartFiberMutex::NeverYield {});
		m_pendingQueue.PushBack(&unit);
	}

	NativeSwitchToFiber(HeartFiberContext::Get().pump);
}

void HeartFiberSystem::NativeSwitchToFiber(HeartFiberWorkUnit& target)
{
	// Get where we're coming from
	HeartFiberWorkUnit* from = HeartFiberContext::Get().currentWorkUnit;
	HeartFiberAbiContext* from_abi = reinterpret_cast<HeartFiberAbiContext*>(from->m_nativeHandle);

	// Populate the context
	HEART_ASSERT(HeartFiberContext::Get().currentSystem == this);
	HeartFiberContext::Get().currentWorkUnit = &target;

	// Execute
	HeartFiberAbiContext* to_abi = reinterpret_cast<HeartFiberAbiContext*>(target.m_nativeHandle);
	heart_swap_fiber_context(from_abi, to_abi);
}

void HeartFiberSystem::NativeSwitchToFiberNoReturn(HeartFiberWorkUnit& target)
{
	// Switch to the fiber. It should never return to us.
	NativeSwitchToFiber(target);

	// Uh oh. the fiber came back. Kill the thread with an error code.
	// It should've exited via the ExitThread in the entry routine
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

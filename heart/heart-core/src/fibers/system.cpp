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

	// Allocate a startup abi so that the system has something to jump away from
	auto* startupAbi = HeartFiberContext::Get().currentSystem->m_allocator.AllocateAndConstruct<HeartFiberAbiContext>();
	HeartFiberContext::Get().entryUnit.m_nativeHandle = startupAbi;
	HeartFiberContext::Get().currentWorkUnit = &HeartFiberContext::Get().entryUnit;

	// Initialize and execute the pump
	HeartFiberContext::Get().currentSystem->InitializeWorkUnitNativeHandle(HeartFiberContext::Get().pump);
	HeartFiberContext::Get().currentSystem->NativeSwitchToFiber(HeartFiberContext::Get().pump);

	// We've returned from the pump, time to cleanup and kill the thread.
	HEART_ASSERT(HeartFiberContext::Get().currentSystem->m_exit == true);
	HeartFiberContext::Get().currentSystem->ReleaseWorkUnitNativeHandle(HeartFiberContext::Get().pump);
	HeartFiberContext::Get().currentSystem->ReleaseWorkUnitNativeHandle(HeartFiberContext::Get().entryUnit);

	return nullptr;
}

void HeartFiberSystem::HeartFiberStartRoutine()
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
		bool destroyEmpty = HeartFiberContext::Get().destroyQueue.IsEmpty();

		bool pendingEmpty;
		{
			HeartLockGuard lock(m_pendingQueueMutex);
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

void HeartFiberSystem::InitializeWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	// We can only create fibers from a fiber!
	HEART_ASSERT(HeartFiberContext::Get().currentSystem != nullptr);

	// TODO: Large stack and small stack fibers
	// SFML requires a fairly deep stack (at least 256kb) so default to large for now
	constexpr uint32_t StackSize = 1u * Meg;
	byte_t* stackStart = m_allocator.allocate<byte_t>(StackSize);

	HeartFiberAbiContext* ctx = m_allocator.AllocateAndConstruct<HeartFiberAbiContext>();
	ctx->allocated = stackStart;

	// Stacks grow downward, point to the end of our buffer, and "allocate"
	// required Windows "red zone" space of 4 64bit values
	byte_t* stackEnd = stackStart + StackSize;
	byte_t* rsp = stackEnd - (4 * sizeof(uint64_t));
	if ((uintptr_t(rsp) & 8L) != 8L)
	{
		// XXX: Windows requires that the stack is always 16-byte aligned... except
		// during the PROLOG of a function, during which it expects it to be "misaligned"
		// by 8 due to the push of the return pointer. It thus always emits code that
		// offsets rsp by 8 + (x * 16). The end result is that if our stack pointer starts
		// off properly aligned, the PROLOG of the StartRoutine misaligns it and causes
		// horrible crashes later. So, we need to purposefully ensure it is *not* 16-byte
		// aligned to start with.
		rsp -= 8;
	}

#if !HEART_STRICT_PERF
	// Zero the whole buffer if we're not in strict perf
	memset(stackStart, 0, StackSize);
#else
	// Zero just the area between our stack pointer and the end if we're in release
	// This isn't strictly necessary, but debugging is already hard enough in
	// optimized builds, we don't need a nonsense garbage stack to contend with too.
	memset(rsp, 0, stackEnd - rsp);
#endif

	ctx->rsp = uintptr_t(rsp);
	ctx->rbp = uintptr_t(rsp);
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

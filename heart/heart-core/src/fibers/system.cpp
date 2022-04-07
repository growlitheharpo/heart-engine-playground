#include "heart/fibers/system.h"

#include "heart/fibers/context.h"
#include "heart/fibers/status.h"
#include "heart/fibers/work_unit.h"

#include "heart/debug/assert.h"

#include "priv/SlimWin32.h"

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

	// This thread is now a fiber
	::ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);

	// Prepare the context for this thread
	HeartFiberContext& threadContext = HeartFiberContext::Get();
	threadContext.currentSystem = currentThread->system;
	threadContext.pump.m_worker = []() { return HeartFiberContext::Get().currentSystem->PumpRoutine(); };

	// Tell the main thread we're done reading the parameter
	currentThread->state = HeartFiberThreadEntryParam::Done;
	currentThread = nullptr;

	// Initialize and execute the pump
	threadContext.currentSystem->InitializeWorkUnitNativeHandle(threadContext.pump);
	threadContext.currentSystem->NativeSwitchToFiber(threadContext.pump);
}

void HeartFiberSystem::HeartFiberStartRoutine(void* parameter)
{
	// Get the work unit and system from the context
	HeartFiberWorkUnit& workUnit = *HeartFiberContext::Get().currentWorkUnit;
	HeartFiberSystem* system = HeartFiberContext::Get().currentSystem;

	// Begin execution of the work unit.
	HeartFiberStatus result = workUnit.m_worker();

	if (&workUnit == &HeartFiberContext::Get().pump)
	{
		// If the work unit was the pump, we're done. This will kill the thread.
		return;
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
			::DeleteFiber(unit->m_nativeHandle);
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

	unit.m_nativeHandle = ::CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, &HeartFiberStartRoutine, NULL);
}

void HeartFiberSystem::CompleteWorkUnit(HeartFiberWorkUnit& unit)
{
	HeartFiberContext::Get().destroyQueue.PushBack(&unit);

	NativeSwitchToFiber(HeartFiberContext::Get().pump);
}

void HeartFiberSystem::RequeueWorkUnit(HeartFiberWorkUnit& unit)
{
	EnqueueFiber(hrt::move(unit.m_worker));

	CompleteWorkUnit(unit);
}

void HeartFiberSystem::YieldUnit(HeartFiberWorkUnit& unit)
{
	{
		HeartLockGuard lock(m_pendingQueueMutex);
		m_pendingQueue.PushBack(&unit);
	}

	NativeSwitchToFiber(HeartFiberContext::Get().pump);
}

void HeartFiberSystem::NativeSwitchToFiber(HeartFiberWorkUnit& target)
{
	// Populate the context
	HEART_ASSERT(HeartFiberContext::Get().currentSystem == this);
	HeartFiberContext::Get().currentWorkUnit = &target;

	// Execute
	::SwitchToFiber(target.m_nativeHandle);
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

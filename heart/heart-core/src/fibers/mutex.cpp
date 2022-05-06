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
#include "heart/fibers/mutex.h"

#include "heart/fibers/context.h"

#include "heart/debug/assert.h"

// TODO: for ultimate perf, these operations probably do not need to be seq_cst
constexpr static std::memory_order ExchangeMemoryOrder = std::memory_order_seq_cst;
constexpr static std::memory_order FenceMemoryOrder = std::memory_order_seq_cst;

void HeartFiberMutex::LockExclusive(YieldToFiber)
{
	while (value.exchange(LockedValue, ExchangeMemoryOrder) != UnlockedValue)
	{
		HeartFiberContext::Yield();
	}
	std::atomic_thread_fence(FenceMemoryOrder);
}

void HeartFiberMutex::LockExclusive(NeverYield)
{
	while (value.exchange(LockedValue, ExchangeMemoryOrder) != UnlockedValue)
	{
	}
	std::atomic_thread_fence(FenceMemoryOrder);
}

bool HeartFiberMutex::TryLockExclusive()
{
	bool success = (value.exchange(LockedValue, ExchangeMemoryOrder) == UnlockedValue);
	std::atomic_thread_fence(FenceMemoryOrder);
	return success;
}

void HeartFiberMutex::Unlock()
{
	HEART_ASSERT(value.load() == LockedValue);
	value.exchange(UnlockedValue, ExchangeMemoryOrder);
}

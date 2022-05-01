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

void HeartFiberMutex::LockExclusive(YieldToFiber)
{
	// TODO: for ultimate perf, these operations probably do not need to be seq_cst

	while (value.exchange(LockedValue) != UnlockedValue)
	{
		HeartFiberContext::Yield();
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
}

void HeartFiberMutex::LockExclusive(NeverYield)
{
	// TODO: for ultimate perf, these operations probably do not need to be seq_cst

	while (value.exchange(LockedValue) != UnlockedValue)
	{
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
}

bool HeartFiberMutex::TryLockExclusive()
{
	// TODO: for ultimate perf, these operations probably do not need to be seq_cst

	bool success = (value.exchange(LockedValue) == UnlockedValue);
	std::atomic_thread_fence(std::memory_order_seq_cst);
	return success;
}

void HeartFiberMutex::Unlock()
{
	HEART_ASSERT(value.load() == LockedValue);
	value.exchange(UnlockedValue);
}

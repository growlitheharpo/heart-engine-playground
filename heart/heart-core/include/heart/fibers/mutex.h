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

#include <heart/types.h>
#include <heart/util/tag_type.h>

#include <atomic>

class HeartFiberMutex
{
public:
	HEART_DECLARE_TAG_TYPE(YieldToFiber);

	HEART_DECLARE_TAG_TYPE(NeverYield);

	HeartFiberMutex() = default;

	void LockExclusive(YieldToFiberT);

	void LockExclusive(NeverYieldT);

	bool TryLockExclusive();

	void Unlock();

private:
	constexpr static int32_t LockedValue = -1;
	constexpr static int32_t UnlockedValue = 0;

	std::atomic_int32_t value = UnlockedValue;
};

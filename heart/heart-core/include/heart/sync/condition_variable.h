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

#include <heart/copy_move_semantics.h>
#include <heart/types.h>

class HeartMutex;

class HeartConditionVariable
{
public:
	enum class WaitOwnership
	{
		Shared,
		Exclusive
	};

private:
	void* m_handle;

public:
	HeartConditionVariable();
	~HeartConditionVariable();

	DISABLE_COPY_AND_MOVE_SEMANTICS(HeartConditionVariable);

	void NotifyOne();
	void NotifyAll();
	void Wait(HeartMutex& mutex, WaitOwnership = WaitOwnership::Exclusive);
	bool TryWaitFor(HeartMutex& mutex, uint32_t milliseconds, WaitOwnership = WaitOwnership::Exclusive);

	void** NativeHandle()
	{
		return &m_handle;
	}
};

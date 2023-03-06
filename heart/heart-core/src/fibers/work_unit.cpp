/* Copyright (C) 2023 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/

#include "heart/fibers/work_unit.h"

#include "heart/allocator.h"

void HeartFiberWorkUnit::IncrementRef() const
{
	++m_useCount;
}

void HeartFiberWorkUnit::DecrementRef() const
{
	if (--m_useCount == 0 && m_allocator != nullptr)
	{
		m_allocator->DestroyAndFree(this);
	}
}

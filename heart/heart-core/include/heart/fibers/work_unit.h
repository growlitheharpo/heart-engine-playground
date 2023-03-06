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

#include <heart/fibers/status.h>
#include <heart/function/embedded_function.h>
#include <heart/memory/intrusive_list.h>
#include <heart/util/tag_type.h>

#include <heart/stl/forward.h>

#include <atomic>

struct HeartBaseAllocator;

class HeartFiberWorkUnit
{
public:
	using WorkerFunction = HeartEmbeddedFunction<HeartFiberResult(), 96>;

private:
	HEART_DECLARE_TAG_TYPE(ConstructorSecret);
	friend class HeartFiberContext;
	friend class HeartFiberSystem;

	HeartFiberWorkUnit() = default;

	void* m_nativeHandle = nullptr;

	HeartIntrusiveListLink m_link = {};

	WorkerFunction m_worker = {};

	mutable std::atomic<HeartFiberWorkUnitStatus> m_status = HeartFiberWorkUnitStatus::Pending;

	mutable std::atomic<uint32> m_useCount = 0;

	HeartBaseAllocator* m_allocator = nullptr;

public:
	HeartFiberWorkUnit(ConstructorSecretT, HeartBaseAllocator* allocator = nullptr) :
		HeartFiberWorkUnit(ConstructorSecretT {}, allocator, WorkerFunction {})
	{
	}

	template <typename F>
	HeartFiberWorkUnit(ConstructorSecretT, HeartBaseAllocator* allocator, F&& f) :
		m_nativeHandle(nullptr),
		m_worker(hrt::forward<F>(f)),
		m_status(HeartFiberWorkUnitStatus::Pending),
		m_useCount(0),
		m_allocator(allocator)
	{
	}

	void IncrementRef() const;

	void DecrementRef() const;

	uint32 GetRefCount() const
	{
		return m_useCount;
	}

	HeartFiberWorkUnitStatus GetStatus() const
	{
		return m_status;
	}

	typedef HeartIntrusiveList<HeartFiberWorkUnit, &HeartFiberWorkUnit::m_link> Queue;
};

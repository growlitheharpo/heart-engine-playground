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

#include <heart/function/embedded_function.h>
#include <heart/memory/intrusive_list.h>

#include <heart/stl/forward.h>

class HeartFiberWorkUnit
{
public:
	using WorkerFunction = HeartEmbeddedFunction<HeartFiberStatus(), 96>;

private:
	struct ConstructorSecretType
	{
	};
	static constexpr ConstructorSecretType ConstructorSecret = {};
	friend class HeartFiberContext;
	friend class HeartFiberSystem;

	HeartFiberWorkUnit() = default;

	void* m_nativeHandle = nullptr;

	HeartIntrusiveListLink m_link = {};

	WorkerFunction m_worker = {};

public:
	HeartFiberWorkUnit(ConstructorSecretType) :
		HeartFiberWorkUnit()
	{
	}

	template <typename F>
	HeartFiberWorkUnit(ConstructorSecretType, F&& f) :
		m_nativeHandle(nullptr),
		m_worker(hrt::forward<F>(f))
	{
	}

	typedef HeartIntrusiveList<HeartFiberWorkUnit, &HeartFiberWorkUnit::m_link> Queue;
};

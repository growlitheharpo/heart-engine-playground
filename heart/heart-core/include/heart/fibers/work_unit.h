#pragma once

#include <heart/fibers/fwd.h>

#include <heart/function.h>
#include <heart/memory/intrusive_list.h>

#include <heart/stl/forward.h>

class HeartFiberWorkUnit
{
public:
	using WorkerFunction = HeartFunction<HeartFiberStatus()>;

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
	template <typename F>
	HeartFiberWorkUnit(ConstructorSecretType, F&& f) :
		m_nativeHandle(nullptr),
		m_worker(hrt::forward<F>(f))
	{
	}

	typedef HeartIntrusiveList<HeartFiberWorkUnit, &HeartFiberWorkUnit::m_link> Queue;
};

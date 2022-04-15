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

#include <heart/config.h>

#include <heart/debug/assert.h>
#include <heart/types.h>

#include <heart/stl/move.h>

#include <heart/util/canonical_typedefs.h>

struct Example
{
	uint32_t refCount = 0;
};

template <typename T>
void HeartIncrementRef(T* t)
{
	t->IncrementRef();
}

template <typename T>
void HeartDecrementRef(T* t)
{
	t->DecrementRef();
}

template <typename T>
uint32_t HeartGetRefCount(T* t)
{
	return t->GetRefCount();
}

template <typename T>
class HeartIntrusivePtr
{
public:
	DECLARE_STANDARD_TYPEDEFS(T);

	HeartIntrusivePtr() = default;

	HeartIntrusivePtr(decltype(nullptr)) noexcept :
		HeartIntrusivePtr()
	{
	}

	template <typename Y>
	HeartIntrusivePtr(Y* ptr) noexcept :
		HeartIntrusivePtr()
	{
		if (ptr)
		{
			_p = ptr;
			HeartIncrementRef(_p);
		}
	}

	HeartIntrusivePtr(HeartIntrusivePtr&& p) noexcept :
		HeartIntrusivePtr()
	{
		p.Swap(*this);
	}

	HeartIntrusivePtr(const HeartIntrusivePtr& p) noexcept :
		HeartIntrusivePtr()
	{
		HeartIntrusivePtr t(p.Get());
		t.Swap(*this);
	}

	HeartIntrusivePtr& operator=(HeartIntrusivePtr&& p) noexcept
	{
		HeartIntrusivePtr(hrt::move(p)).Swap(*this);
		return *this;
	}

	HeartIntrusivePtr& operator=(const HeartIntrusivePtr& p) noexcept
	{
		HeartIntrusivePtr(p).Swap(*this);
		return *this;
	}

	~HeartIntrusivePtr() noexcept
	{
		Reset();
	}

	void Reset() noexcept
	{
		if (_p)
		{
			HeartDecrementRef(_p);
			_p = nullptr;
		}
	}

	void Swap(HeartIntrusivePtr& p) noexcept
	{
		pointer t = p.Get();
		p._p = Get();
		_p = t;
	}

	pointer Get() const
	{
		return _p;
	}

	pointer operator->() const
	{
		return _p;
	}

	reference operator*() const
	{
		HEART_ASSERT(_p);
		return *_p;
	}

	uint32_t UseCount() const
	{
		if (!_p)
			return 0;

		return HeartGetRefCount(_p);
	}

	explicit operator bool() const noexcept
	{
		return _p != nullptr;
	}

	template <typename U>
	friend bool operator==(const HeartIntrusivePtr& lhs, const HeartIntrusivePtr<U>& rhs) noexcept
	{
		return lhs.Get() == rhs.Get();
	}

	template <typename U>
	friend auto operator<=>(const HeartIntrusivePtr& lhs, const HeartIntrusivePtr<U>& rhs) noexcept
	{
		return lhs.Get() <=> rhs.Get();
	}

	friend bool operator==(const HeartIntrusivePtr& lhs, decltype(nullptr)) noexcept
	{
		return lhs.Get() == nullptr;
	}

	friend auto operator<=>(const HeartIntrusivePtr& lhs, decltype(nullptr)) noexcept
	{
		return lhs.Get() <=> nullptr;
	}

private:
	pointer _p = nullptr;
};

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

#include <heart/function/details_function_base.h>

#include <heart/allocator.h>

#include <heart/debug/assert.h>

template <typename Sig>
class HeartDynamicFunction;

template <typename R, typename... Args>
class HeartDynamicFunction<R(Args...)>
{
private:
	using BaseType = heart_priv::HeartFunctionBase<R, Args...>;

	template <typename F>
	using ImplType = heart_priv::HeartFunctionImpl<hrt::remove_reference_t<F>, R, Args...>;

	BaseType* m_pointer = nullptr;
	HeartBaseAllocator& m_allocator;

public:
	HeartDynamicFunction(HeartBaseAllocator& a = GetHeartDefaultAllocator()) :
		m_allocator(a)
	{
	}

	DISABLE_COPY_SEMANTICS(HeartDynamicFunction);

	HeartDynamicFunction(HeartDynamicFunction&& o) :
		HeartDynamicFunction(o.m_allocator)
	{
		Swap(o);
	}

	HeartDynamicFunction& operator=(HeartDynamicFunction&& o)
	{
		Swap(o);
		return *this;
	}

	template <typename F>
	HeartDynamicFunction(F f, HeartBaseAllocator& a = GetHeartDefaultAllocator()) :
		HeartDynamicFunction(a)
	{
		Set(hrt::forward<F>(f));
	}

	template <typename F>
	void Set(F& f)
	{
		m_pointer = m_allocator.AllocateAndConstruct<ImplType<F>>(hrt::forward<F>(f));
	}

	template <typename F>
	void Set(F&& f)
	{
		m_pointer = m_allocator.AllocateAndConstruct<ImplType<F>>(hrt::forward<F>(f));
	}

	void Clear()
	{
		if (m_pointer != nullptr)
		{
			m_allocator.DestroyAndFree(m_pointer);
			m_pointer = nullptr;
		}
	}

	bool IsSet() const
	{
		return m_pointer != nullptr;
	}

	void Swap(HeartDynamicFunction& o)
	{
		if (&this->m_allocator == &o.m_allocator)
		{
			// Same allocator, just a pointer swap
			BaseType* tmp = m_pointer;
			m_pointer = o.m_pointer;
			o.m_pointer = tmp;
		}
		else
		{
			BaseType* tPtr = m_pointer;
			BaseType* oPtr = o.m_pointer;
			m_pointer = nullptr;
			o.m_pointer = nullptr;

			auto movePointer = [](BaseType*& target, BaseType*& source, HeartBaseAllocator& targetAllocator, HeartBaseAllocator& sourceAllocator) {
				void* newStorage = targetAllocator.RawAllocate(source->GetSize());
				target = source->Move(newStorage);
				sourceAllocator.DestroyAndFree(source);
				source = nullptr;
			};

			if (tPtr)
			{
				movePointer(o.m_pointer, tPtr, o.m_allocator, m_allocator);
			}

			if (oPtr)
			{
				movePointer(m_pointer, oPtr, m_allocator, o.m_allocator);
			}
		}
	}

	explicit operator bool() const
	{
		return IsSet();
	}

	R operator()(Args... args)
	{
		HEART_ASSERT(IsSet());
		return m_pointer->Call(hrt::forward<Args>(args)...);
	}
};

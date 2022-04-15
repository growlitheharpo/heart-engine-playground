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

#include <heart/copy_move_semantics.h>

#include <heart/stl/forward.h>
#include <heart/stl/type_traits/add_remove_ref_cv.h>
#include <heart/stl/type_traits/aligned_storage.h>
#include <heart/stl/type_traits/enable_if.h>

#include <heart/debug/assert.h>

template <typename Sig, size_t Storage = 32>
class HeartEmbeddedFunction;

template <typename R, typename... Args, size_t Storage>
class HeartEmbeddedFunction<R(Args...), Storage>
{
private:
	using BaseType = heart_priv::HeartFunctionBase<R, Args...>;

	using StorageType = hrt::aligned_storage_t<Storage>;

	template <typename F>
	using ImplType = heart_priv::HeartFunctionImpl<hrt::remove_reference_t<F>, R, Args...>;

	bool m_set = false;
	StorageType m_storage = {};

	BaseType* GetPtr()
	{
		return reinterpret_cast<BaseType*>(&m_storage);
	}

	template <typename F>
	static constexpr bool SizeCheck = (sizeof(ImplType<F>) <= sizeof(m_storage));

public:
	HeartEmbeddedFunction() = default;

	~HeartEmbeddedFunction()
	{
		Clear();
	}

	DISABLE_COPY_SEMANTICS(HeartEmbeddedFunction);

	HeartEmbeddedFunction(HeartEmbeddedFunction&& o) :
		HeartEmbeddedFunction()
	{
		Swap(o);
	}

	HeartEmbeddedFunction& operator=(HeartEmbeddedFunction&& o)
	{
		Swap(o);
		return *this;
	}

	template <typename F, hrt::enable_if_t<SizeCheck<F>, void*> = nullptr>
	HeartEmbeddedFunction(F f)
	{
		Set(hrt::forward<F>(f));
	}

	void Swap(HeartEmbeddedFunction& other)
	{
		if (m_set != other.m_set)
		{
			HeartEmbeddedFunction* wasSet;
			HeartEmbeddedFunction* toSet;

			if (m_set)
			{
				wasSet = this;
				toSet = &other;
			}
			else
			{
				wasSet = &other;
				toSet = this;
			}

			wasSet->GetPtr()->Move(toSet->GetPtr());
			wasSet->Clear();
			toSet->m_set = true;
		}
		else
		{
			HeartEmbeddedFunction tmp;

			// Put "other" in tmp
			tmp.Swap(other);

			// Put this in other
			other.Swap(*this);

			// Put "other" in this (via tmp)
			tmp.Swap(*this);
		}
	}

	template <typename F, hrt::enable_if_t<SizeCheck<F>, void*> = nullptr>
	void Set(F& f)
	{
		new (GetPtr()) ImplType<F>(hrt::forward<F>(f));
		m_set = true;
	}

	template <typename F, hrt::enable_if_t<SizeCheck<F>, void*> = nullptr>
	void Set(F&& f)
	{
		new (GetPtr()) ImplType<F>(hrt::forward<F>(f));
		m_set = true;
	}

	void Clear()
	{
		if (m_set)
		{
			GetPtr()->~BaseType();
			m_set = false;
			for (char* p = (char*)&m_storage; p < (char*)&m_storage + sizeof(m_storage); ++p)
				*p = 0;
		}
	}

	explicit operator bool() const
	{
		return m_set;
	}

	R operator()(Args... args)
	{
		HEART_ASSERT(m_set);
		return GetPtr()->Call(hrt::forward<Args>(args)...);
	}
};

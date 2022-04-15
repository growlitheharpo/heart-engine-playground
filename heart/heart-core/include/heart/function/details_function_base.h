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

#include <heart/stl/forward.h>
#include <heart/stl/move.h>
#include <heart/stl/type_traits/add_remove_ref_cv.h>
#include <heart/stl/type_traits/same.h>

namespace heart_priv
{
	template <typename R, typename... Args>
	class HeartFunctionBase
	{
	public:
		virtual R Call(Args&&... args) = 0;
		virtual HeartFunctionBase* Move(void* location) = 0;
		virtual ~HeartFunctionBase() = default;
		virtual size_t GetSize() const = 0;
	};

	template <typename F, typename R, typename... Args>
	class HeartFunctionImpl : public HeartFunctionBase<R, Args...>
	{
		static_assert(hrt::is_same_v<F, hrt::remove_reference_t<F>>);
		F f;

	public:
		HeartFunctionImpl(F&& f) :
			f(hrt::forward<F>(f))
		{
		}

		~HeartFunctionImpl() = default;

		R Call(Args&&... args) override
		{
			return f(hrt::forward<Args>(args)...);
		}

		HeartFunctionBase<R, Args...>* Move(void* location) override
		{
			auto* p = new (location) HeartFunctionImpl<F, R, Args...>(hrt::move(f));
			return p;
		}

		size_t GetSize() const override
		{
			return sizeof(HeartFunctionImpl<F, R, Args...>);
		}
	};
}

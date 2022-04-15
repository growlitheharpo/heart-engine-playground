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

#include <heart/stl/type_traits/constants.h>
#include <heart/stl/type_traits/void.h>

namespace hrt
{
	template <class _Ty>
	inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(_Ty);

	template <typename From, typename To>
	inline constexpr bool is_convertible_v = __is_convertible_to(From, To);

	template <typename From, typename To>
	struct is_convertible : bool_constant<is_convertible_v<From, To>>
	{
	};

	namespace details
	{
		template <class To>
		void can_implicitly_convert_to(To) noexcept;
	}

	template <class From, class To, bool = is_convertible_v<From, To>, bool = is_void_v<To>>
	constexpr bool is_nothrow_convertible_v = noexcept(details::can_implicitly_convert_to<To>(declval<From>()));

	template <class From, class To, bool IsVoid>
	constexpr bool is_nothrow_convertible_v<From, To, false, IsVoid> = false;

	template <class From, class To>
	constexpr bool is_nothrow_convertible_v<From, To, true, true> = true;

	template <class From, class To>
	struct is_nothrow_convertible : bool_constant<is_nothrow_convertible_v<From, To>>
	{
		// determine whether From is nothrow-convertible to To
	};
}

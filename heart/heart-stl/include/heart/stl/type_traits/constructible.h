#pragma once

#include <heart/stl/type_traits/constants.h>

namespace hrt
{
	// Boooo, having to use MSVC extensions for these...

	template <class T, class... Args>
	struct is_constructible : bool_constant<__is_constructible(T, Args...)>
	{
	};

	template <class T, class... Args>
	inline constexpr bool is_constructible_v = __is_constructible(T, Args...);

	template <class T>
	struct is_copy_constructible : bool_constant<__is_constructible(T, add_lvalue_reference_t<const T>)>
	{
	};

	template <class T>
	inline constexpr bool is_copy_constructible_v = __is_constructible(T, add_lvalue_reference_t<const T>);

	template <class T>
	struct is_default_constructible : bool_constant<__is_constructible(T)>
	{
	};

	template <class T>
	inline constexpr bool is_default_constructible_v = __is_constructible(T);

	template <class T>
	struct is_move_constructible : bool_constant<__is_constructible(T, T)>
	{
	};

	template <class T>
	inline constexpr bool is_move_constructible_v = __is_constructible(T, T);
}

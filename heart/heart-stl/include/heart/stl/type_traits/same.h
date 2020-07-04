#pragma once

#include <heart/stl/type_traits/constants.h>

namespace hrt
{
	template <typename T1, typename T2>
	inline constexpr bool is_same_v = false;

	template <typename T>
	inline constexpr bool is_same_v<T, T> = true;

	template <typename T1, typename T2>
	struct is_same : bool_constant<is_same_v<T1, T2>>
	{
	};
}

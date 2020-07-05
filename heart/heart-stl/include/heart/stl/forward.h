#pragma once

#include <heart/stl/type_traits.h>

namespace hrt
{
	template <typename T>
	constexpr T&& forward(remove_reference_t<T>& x) noexcept
	{
		return static_cast<T&&>(x);
	}

	template <typename T>
	constexpr T&& forward(remove_reference_t<T>&& x) noexcept
	{
		return static_cast<T&&>(x);
	}
}

#pragma once

#include <heart/stl/type_traits.h>

namespace hrt
{
	template <typename T>
	constexpr remove_reference_t<T>&& move(T&& x) noexcept
	{
		return static_cast<remove_reference_t<T>&&>(x);
	}
}

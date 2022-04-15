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

#include <heart/stl/type_traits.h>

namespace hrt
{
	template <typename T>
	constexpr remove_reference_t<T>&& move(T&& x) noexcept
	{
		return static_cast<remove_reference_t<T>&&>(x);
	}
}

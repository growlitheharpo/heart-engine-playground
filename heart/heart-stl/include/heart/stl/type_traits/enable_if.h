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

#include <heart/stl/type_traits/add_remove_ref_cv.h>

namespace hrt
{
	template <class T>
	add_rvalue_reference_t<T> declval() noexcept;

	template <bool Test, class T = void>
	struct enable_if
	{
	}; // no member "type" when !Test

	template <class T>
	struct enable_if<true, T>
	{
		using type = T;
	};

	template <bool Test, class T = void>
	using enable_if_t = typename enable_if<Test, T>::type;
}

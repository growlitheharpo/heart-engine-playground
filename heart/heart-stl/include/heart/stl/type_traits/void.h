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
#include <heart/stl/type_traits/constants.h>
#include <heart/stl/type_traits/same.h>

namespace hrt
{
	// void_t itself comes from add_remove_ref_cv

	// STRUCT TEMPLATE is_void
	template <typename T>
	constexpr bool is_void_v = is_same_v<remove_cv_t<T>, void>;

	template <typename T>
	struct is_void : bool_constant<is_void_v<T>>
	{
	};
}

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

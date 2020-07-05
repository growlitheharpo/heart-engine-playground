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

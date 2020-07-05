#pragma once

#include <heart/config.h>

#include <heart/stl/type_traits/add_remove_ref_cv.h>
#include <heart/stl/type_traits/conditional.h>
#include <heart/stl/type_traits/constants.h>
#include <heart/stl/type_traits/constructible.h>
#include <heart/stl/type_traits/convertible.h>
#include <heart/stl/type_traits/enable_if.h>
// #include <heart/stl/type_traits/invoke.h> // not ready yet
#include <heart/stl/type_traits/pointer.h>
#include <heart/stl/type_traits/same.h>
#include <heart/stl/type_traits/void.h>

namespace hrt
{
	template <class _Ty>
	struct is_trivially_copyable : bool_constant<__is_trivially_copyable(_Ty)>
	{
	};
}

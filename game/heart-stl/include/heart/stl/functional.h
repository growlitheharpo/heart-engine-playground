#pragma once

#include <heart/config.h>

#define HEART_STD_FUNC 1

#if HEART_IS_STD || HEART_STD_FUNC
#include <functional>
#endif

namespace hrt
{
#if HEART_IS_STD || HEART_STD_FUNC
	using namespace std;
#else
	static_assert(false, "hrt::functional is not implemented!");
#endif
}

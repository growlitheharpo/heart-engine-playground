#pragma once

#include <heart/config.h>
#include <heart/types.h>

#define USE_STD_UTILITY 1

#if HEART_IS_STD || USE_STD_UTILITY
#include <utility>
#endif

namespace hrt
{
#if HEART_IS_STD || USE_STD_UTILITY
	using namespace std;
#else
	static_assert(false, "hrt::utility is not implemented!");
#endif
}

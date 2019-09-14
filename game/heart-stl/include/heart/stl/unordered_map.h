#pragma once

#include <heart/types.h>
#include <heart/stl/util/config.h>

#define USE_STD_UNORDERED_MAP 1

#if HEART_IS_STD || USE_STD_UNORDERED_MAP
#include <unordered_map>
#endif

namespace hrt
{
#if HEART_IS_STD || USE_STD_UNORDERED_MAP
	template <typename T1, typename T2>
	using unordered_map = std::unordered_map<T1, T2>;
#else
	static_assert(false, "hrt::unorered_map<> is not implemented!");
#endif
}

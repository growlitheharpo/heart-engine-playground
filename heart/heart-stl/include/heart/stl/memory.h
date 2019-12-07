#pragma once

#include <heart/config.h>

#define HEART_STD_MEMORY 1

#if HEART_IS_STD || HEART_STD_MEMORY
#include <memory>
#endif

namespace hrt
{
#if HEART_IS_STD || HEART_STD_MEMORY
	using namespace std;
#else
	static_assert(false, "hrt::memory is not implemented!");
#endif
}

#pragma once

#include <heart/types.h>
#include <heart/config.h>

#define USE_STD_STRING 1

#if HEART_IS_STD || USE_STD_STRING
#include <string>
#endif

namespace hrt
{
#if HEART_IS_STD || USE_STD_STRING
	using string = std::string;
#else
	static_assert(false, "hrt::string is not implemented!");
#endif
}

size_t WriteDebugValue(char* tgt, size_t len, const hrt::string& value);

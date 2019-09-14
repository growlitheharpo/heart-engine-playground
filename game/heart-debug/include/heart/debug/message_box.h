#pragma once

#include <heart/types.h>

bool DisplayAssertError(const char* title, const char* expr, const char* msg, const char* file, uint32_t line, bool* ignoreAlways);

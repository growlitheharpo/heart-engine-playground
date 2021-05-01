#pragma once

#include <cassert>

#define HEART_ASSERT(expression, ...) assert(expression) //(void)0
#define HEART_CHECK(expr, ...) (!!(expr))

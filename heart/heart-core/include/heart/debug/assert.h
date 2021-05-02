#pragma once

#include <cassert>

#define HEART_ASSERT(expression, ...) assert(expression)
#define HEART_CHECK(expr, ...) (!!(expr))

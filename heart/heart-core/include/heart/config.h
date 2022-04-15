#pragma once

#if !defined(HEART_STRICT_PERF)
#if defined(NDEBUG)
#define HEART_STRICT_PERF 1
#else
#define HEART_STRICT_PERF 0
#endif
#endif

#if !defined(HEART_INCLUDE_DEBUG_STRINGS)
#if defined(NDEBUG)
#define HEART_INCLUDE_DEBUG_STRINGS 0
#else
#define HEART_INCLUDE_DEBUG_STRINGS 1
#endif
#endif

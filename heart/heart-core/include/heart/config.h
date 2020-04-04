#pragma once

#if !defined(HEART_STRICT_PERF)
#if defined(NDEBUG)
#define HEART_STRICT_PERF 1
#else
#define HEART_STRICT_PERF 0
#endif
#endif

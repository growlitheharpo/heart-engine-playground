/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
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

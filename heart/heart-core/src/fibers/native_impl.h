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

#include "heart/config.h"

#if !HEART_STRICT_PERF && !HEART_USE_OS_FIBERS
extern "C" void heart_verify_stack_pointer();
#define HEART_FIBER_VERIFY_STACK() heart_verify_stack_pointer()
#else
#define HEART_FIBER_VERIFY_STACK() ((void)0)
#endif

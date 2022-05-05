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

#include "heart/config.h"
#include "heart/types.h"

#include <xmmintrin.h>

struct HeartFiberAbiContext
{
	uintptr_t rip = 0;
	uintptr_t rsp = 0;
	uintptr_t rbx = 0;
	uintptr_t rbp = 0;
	uintptr_t r12 = 0;
	uintptr_t r13 = 0;
	uintptr_t r14 = 0;
	uintptr_t r15 = 0;
	uintptr_t rdi = 0;
	uintptr_t rsi = 0;
	__m128 xmm6 = {};
	__m128 xmm7 = {};
	__m128 xmm8 = {};
	__m128 xmm9 = {};
	__m128 xmm10 = {};
	__m128 xmm11 = {};
	__m128 xmm12 = {};
	__m128 xmm13 = {};
	__m128 xmm14 = {};
	__m128 xmm15 = {};

	// Extra
	byte_t* allocated = nullptr;
};

extern "C" void heart_swap_fiber_context(HeartFiberAbiContext* from, HeartFiberAbiContext* to);
extern "C" void heart_verify_stack_pointer();

#if !HEART_STRICT_PERF
#define HEART_FIBER_VERIFY_STACK() heart_verify_stack_pointer()
#else
#define HEART_FIBER_VERIFY_STACK() ((void)0)
#endif

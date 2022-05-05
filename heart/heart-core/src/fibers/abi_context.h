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
	uintptr_t rip;
	uintptr_t rsp;
	uintptr_t rbx;
	uintptr_t rbp;
	uintptr_t r12;
	uintptr_t r13;
	uintptr_t r14;
	uintptr_t r15;
	uintptr_t rdi;
	uintptr_t rsi;
	__m128 xmm6;
	__m128 xmm7;
	__m128 xmm8;
	__m128 xmm9;
	__m128 xmm10;
	__m128 xmm11;
	__m128 xmm12;
	__m128 xmm13;
	__m128 xmm14;
	__m128 xmm15;

	// Extra
	byte_t* allocated;
};

extern "C" void heart_swap_fiber_context(HeartFiberAbiContext* from, HeartFiberAbiContext* to);
extern "C" void heart_verify_stack_pointer();

#if !HEART_STRICT_PERF
#define HEART_FIBER_VERIFY_STACK heart_verify_stack_pointer
#else
#define HEART_FIBER_VERIFY_STACK do {} while(false)
#endif

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
#include "fibers/native_impl.h"

#include "heart/fibers/context.h"
#include "heart/fibers/status.h"
#include "heart/fibers/system.h"
#include "heart/fibers/work_unit.h"

#include "heart/allocator.h"
#include "heart/debug/assert.h"
#include "heart/fibers/work_unit.h"

#include <stdio.h>

// TODO: Large stack and small stack fibers
// SFML requires a fairly deep stack (at least 256kb) so default to large for now
constexpr uint32_t StackSize = 1u * Meg;

#if !HEART_USE_OS_FIBERS
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

void HeartFiberSystem::InitializeEntryFiber()
{
	auto* startupAbi = m_allocator.AllocateAndConstruct<HeartFiberAbiContext>();
	HeartFiberContext::Get().entryUnit.m_nativeHandle = startupAbi;
}

void HeartFiberSystem::ReleaseEntryFiber()
{
	ReleaseWorkUnitNativeHandle(HeartFiberContext::Get().entryUnit);
}

void HeartFiberSystem::InitializeWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	// We can only create fibers from a fiber!
	HEART_ASSERT(HeartFiberContext::Get().currentSystem != nullptr);

	// TODO: Large stack and small stack fibers
	// SFML requires a fairly deep stack (at least 256kb) so default to large for now
	byte_t* stackStart = m_allocator.allocate<byte_t>(StackSize);

	HeartFiberAbiContext* ctx = m_allocator.AllocateAndConstruct<HeartFiberAbiContext>();
	ctx->allocated = stackStart;

	// Stacks grow downward, point to the end of our buffer, and "allocate"
	// required Windows "red zone" space of 4 64bit values
	byte_t* stackEnd = stackStart + StackSize;
	byte_t* rsp = stackEnd - (4 * sizeof(uint64_t));
	if ((uintptr_t(rsp) & 8L) != 8L)
	{
		// XXX: Windows requires that the stack is always 16-byte aligned... except
		// during the PROLOG of a function, during which it expects it to be "misaligned"
		// by 8 due to the push of the return pointer. It thus always emits code that
		// offsets rsp by 8 + (x * 16). The end result is that if our stack pointer starts
		// off properly aligned, the PROLOG of the StartRoutine misaligns it and causes
		// horrible crashes later. So, we need to purposefully ensure it is *not* 16-byte
		// aligned to start with.
		rsp -= 8;
	}

#if !HEART_STRICT_PERF
	// Zero the whole buffer if we're not in strict perf
	memset(stackStart, 0, StackSize);
#else
	// Zero just the area between our stack pointer and the end if we're in release
	// This isn't strictly necessary, but debugging is already hard enough in
	// optimized builds, we don't need a nonsense garbage stack to contend with too.
	memset(rsp, 0, stackEnd - rsp);
#endif

	ctx->rsp = uintptr_t(rsp);
	ctx->rbp = uintptr_t(rsp);
	ctx->rip = uintptr_t(&HeartFiberStartRoutine);

	unit.m_nativeHandle = ctx;
}

void HeartFiberSystem::ReleaseWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	HEART_ASSERT(HeartFiberContext::Get().currentSystem != nullptr);

	HeartFiberAbiContext* ctx = reinterpret_cast<HeartFiberAbiContext*>(unit.m_nativeHandle);
	unit.m_nativeHandle = nullptr;

	// Free the stack
	m_allocator.deallocate(ctx->allocated);
	ctx->allocated = nullptr;

	// Free the actual abi context
	m_allocator.DestroyAndFree<HeartFiberAbiContext>(ctx);
	ctx = nullptr;
}

void HeartFiberSystem::NativeSwitchToFiber(HeartFiberWorkUnit& target)
{
	// Get where we're coming from
	HeartFiberWorkUnit* from = HeartFiberContext::Get().currentWorkUnit;
	HeartFiberAbiContext* from_abi = reinterpret_cast<HeartFiberAbiContext*>(from->m_nativeHandle);

	// Populate the context
	HEART_ASSERT(HeartFiberContext::Get().currentSystem == this);
	HeartFiberContext::Get().currentWorkUnit = &target;

	// Execute
	HeartFiberAbiContext* to_abi = reinterpret_cast<HeartFiberAbiContext*>(target.m_nativeHandle);
	heart_swap_fiber_context(from_abi, to_abi);
}

#else

#include "priv/SlimWin32.h"

void HeartFiberSystem::InitializeEntryFiber()
{
	void* nativeStartupHandle = ::ConvertThreadToFiberEx(NULL, 0);
	HeartFiberContext::Get().entryUnit.m_nativeHandle = nativeStartupHandle;
}

void HeartFiberSystem::ReleaseEntryFiber()
{
	// Do nothing - Windows will clean it up for us when we fall off the entry function
	// If we were to destroy it, we'd crash (since it's what's currently executing).
}

void HeartFiberSystem::InitializeWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	// We can only create fibers from a fiber!
	HEART_ASSERT(HeartFiberContext::Get().currentSystem != nullptr);

	unit.m_nativeHandle = ::CreateFiberEx(StackSize, StackSize, 0, &HeartFiberStartRoutine, NULL);
}

void HeartFiberSystem::ReleaseWorkUnitNativeHandle(HeartFiberWorkUnit& unit)
{
	::DeleteFiber(unit.m_nativeHandle);
	unit.m_nativeHandle = nullptr;
}

void HeartFiberSystem::NativeSwitchToFiber(HeartFiberWorkUnit& target)
{
	// Populate the context
	HEART_ASSERT(HeartFiberContext::Get().currentSystem == this);
	HeartFiberContext::Get().currentWorkUnit = &target;

	// Execute
	::SwitchToFiber(target.m_nativeHandle);
}

#endif

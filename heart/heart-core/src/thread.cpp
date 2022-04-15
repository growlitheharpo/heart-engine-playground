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
#include "heart/thread.h"

#include "heart/debug/assert.h"

#include <atomic>
#include <malloc.h>

#include "priv/SlimWin32.h"

struct Passthrough
{
	HeartThread::EntryPoint f = nullptr;
	void* u = nullptr;
};

// TODO: this is not exactly safe
static Passthrough s_passthroughPool[256] = {};
static std::atomic<uint8_t> s_passthroughIndex;

static DWORD WINAPI GlobalThreadEntryPoint(LPVOID param)
{
	uint8_t index = (uint8_t)(uintptr_t)param;
	Passthrough& pass = s_passthroughPool[index];

	void* r = pass.f(pass.u);

	return r == nullptr ? 0 : 1;
}

const int PriorityRemap[] = {
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_TIME_CRITICAL,
};

HeartThread::HeartThread(EntryPoint entryPoint, void* userData, Priority priority, uint32_t stackSize)
{
	uint8_t index = s_passthroughIndex.fetch_add(1);
	Passthrough& pass = s_passthroughPool[index];
	pass.f = entryPoint;
	pass.u = userData;

	m_handle = ::CreateThread(
		nullptr,
		stackSize,
		&GlobalThreadEntryPoint,
		(void*)index,
		0,
		nullptr);

	if (m_handle)
	{
		SetThreadPriority(m_handle, PriorityRemap[(int)priority]);
	}
}

HeartThread::~HeartThread()
{
	HEART_ASSERT(!m_handle, "Thread is being destroyed without being explicitly joined or detached!");
	if (m_handle)
	{
		Detach();
	}
}

HeartThread::HeartThread(HeartThread&& o) noexcept
{
	this->m_handle = o.m_handle;
	o.m_handle = nullptr;
}

HeartThread& HeartThread::operator=(HeartThread&& o) noexcept
{
	this->m_handle = o.m_handle;
	o.m_handle = nullptr;
	return *this;
}

void HeartThread::Join()
{
	HANDLE h = m_handle;
	m_handle = nullptr;

	if (h)
	{
		WaitForSingleObject(h, INFINITE);
	}
}

void HeartThread::Detach()
{
	HANDLE h = m_handle;
	m_handle = nullptr;

	if (h)
	{
		CloseHandle(h);
	}
}

void HeartThread::SetName(const char* name)
{
	auto wideLength = ::MultiByteToWideChar(0, 0, name, -1, NULL, 0);
	wchar_t* buffer = (wchar_t*)_alloca(wideLength * sizeof(wchar_t));
	::MultiByteToWideChar(0, 0, name, -1, buffer, wideLength);
	::SetThreadDescription(HANDLE(m_handle), buffer);
}

HeartThread::operator bool() const
{
	return (m_handle != nullptr);
}

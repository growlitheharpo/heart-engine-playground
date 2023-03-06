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

#include <heart/io/io_forward_decl.h>

#include <heart/sync/condition_variable.h>
#include <heart/sync/mutex.h>
#include <heart/thread/thread.h>

#include <heart/stl/vector.h>

#include <atomic>

class IoCmdQueue
{
public:
	IoCmdQueue(int threadCount = 1);
	~IoCmdQueue();

	void Submit(IoCmdList* cmdList);
	void Flush();
	void Close();

private:
	hrt::vector<HeartThread> m_threads;
	HeartMutex m_mutex;
	HeartConditionVariable m_cv;

	void ThreadThink();

	std::atomic_bool m_threadExit = false;

	struct CmdPage
	{
		CmdPage* next = nullptr;
		uint16_t size = 0;
		uint16_t inUse = 0;
		uint8_t data[8ull * Kilo];
	};

	CmdPage m_pages[16] = {};
	CmdPage* m_head = nullptr;

	void ProcessCmdPage(CmdPage& page);
};

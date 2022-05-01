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
#include <heart/fibers/context.h>
#include <heart/fibers/status.h>
#include <heart/fibers/system.h>

#include <gtest/gtest.h>

#include "utils/tracking_allocator.h"

#include <mutex>
#include <thread>
#include <vector>

TEST(Fibers, BasicStartupShutdown)
{
	HeartFiberSystem system;

	system.Initialize({});
	system.Shutdown();
}

TEST(Fibers, SingleWorkUnit)
{
	HeartFiberSystem system;

	system.Initialize({});

	static int32_t someData = 0;

	system.EnqueueFiber([]() {
		EXPECT_EQ(someData, 0);
		someData = 1;
		return HeartFiberStatus::Complete;
	});

	system.Shutdown();

	EXPECT_EQ(someData, 1);
}

TEST(Fibers, MultiWorkUnit)
{
	HeartFiberSystem system;
	system.Initialize({});

	static int32_t someData = 0;

	auto worker = []() {
		++someData;
		return HeartFiberStatus::Complete;
	};

	system.EnqueueFiber(worker);
	system.EnqueueFiber(worker);
	system.EnqueueFiber(worker);
	system.EnqueueFiber(worker);
	system.EnqueueFiber(worker);

	system.Shutdown();

	EXPECT_EQ(someData, 5);
}

TEST(Fibers, YieldingWorkUnit)
{
	HeartFiberSystem system;
	system.Initialize({});

	static std::atomic_bool job2Queued = false;

	enum State
	{
		None,
		Job1Waiting,
		Job1Yielding,
		Job2Yielding,
		Job1Finished,
		Job2Finished,
	};

	static State state = None;

	system.EnqueueFiber([]() {
		EXPECT_EQ(state, None);
		state = Job1Waiting;

		while (!job2Queued)
			std::this_thread::sleep_for(std::chrono::milliseconds(5));

		state = Job1Yielding;
		HeartFiberContext::Yield();

		EXPECT_EQ(state, Job2Yielding);

		state = Job1Finished;
		return HeartFiberStatus::Complete;
	});

	system.EnqueueFiber([]() {
		EXPECT_EQ(state, Job1Yielding);

		state = Job2Yielding;
		HeartFiberContext::Yield();

		state = Job2Finished;
		return HeartFiberStatus::Complete;
	});
	job2Queued = true;

	system.Shutdown();
	EXPECT_EQ(state, Job2Finished);
}

TEST(Fibers, Requeue)
{
	HeartFiberSystem system;
	system.Initialize({});

	std::atomic<int32_t> counter = 0;
	system.EnqueueFiber([&]() {
		if (++counter < 5)
		{
			return HeartFiberStatus::Requeue;
		}

		return HeartFiberStatus::Complete;
	});

	system.Shutdown();
	EXPECT_EQ(counter, 5);
}

TEST(Fibers, FourThreads)
{
	HeartFiberSystem::Settings settings;
	settings.threadCount = 4;

	HeartFiberSystem system;
	system.Initialize(settings);

	const int JobCount = 128;
	std::atomic_int doneCount = 0;

	HeartFiberMutex threadIdsMutex;
	std::vector<std::thread::id> threadIds;

	int i = JobCount;
	while (i--)
	{
		system.EnqueueFiber([&]() {
			{
				HeartLockGuard lock(threadIdsMutex, HeartFiberMutex::NeverYield {});
				if (std::find(threadIds.begin(), threadIds.end(), std::this_thread::get_id()) == threadIds.end())
				{
					threadIds.push_back(std::this_thread::get_id());
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			doneCount++;

			return HeartFiberStatus::Complete;
		});
	}

	while (doneCount < JobCount)
		std::this_thread::yield();

	// With so many long-running jobs, we expect every thread to have been hit
	EXPECT_EQ(threadIds.size(), settings.threadCount);

	// And they should've all finished
	EXPECT_EQ(doneCount, JobCount);

	system.Shutdown();
}

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
#include <heart/fibers/mutex.h>
#include <heart/fibers/result.h>
#include <heart/fibers/system.h>

#include <heart/sync/mutex.h>

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

	system.EnqueueWork([]() {
		EXPECT_EQ(someData, 0);
		someData = 1;
		return HeartFiberResult::Success;
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
		return HeartFiberResult::Success;
	};

	system.EnqueueWork(worker);
	system.EnqueueWork(worker);
	system.EnqueueWork(worker);
	system.EnqueueWork(worker);
	system.EnqueueWork(worker);

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

	system.EnqueueWork([]() {
		EXPECT_EQ(state, None);
		state = Job1Waiting;

		while (!job2Queued)
			std::this_thread::sleep_for(std::chrono::milliseconds(5));

		state = Job1Yielding;
		HeartFiberContext::Yield();

		EXPECT_EQ(state, Job2Yielding);

		state = Job1Finished;
		return HeartFiberResult::Success;
	});

	system.EnqueueWork([]() {
		EXPECT_EQ(state, Job1Yielding);

		state = Job2Yielding;
		HeartFiberContext::Yield();

		state = Job2Finished;
		return HeartFiberResult::Success;
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
	system.EnqueueWork([&]() {
		if (++counter < 5)
		{
			return HeartFiberResult::Retry;
		}

		return HeartFiberResult::Success;
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

	std::mutex threadIdsMutex;
	std::vector<std::thread::id> threadIds;

	int i = JobCount;
	while (i--)
	{
		system.EnqueueWork([&]() {
			{
				std::lock_guard lock(threadIdsMutex);
				if (std::find(threadIds.begin(), threadIds.end(), std::this_thread::get_id()) == threadIds.end())
				{
					threadIds.push_back(std::this_thread::get_id());
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			doneCount++;

			return HeartFiberResult::Success;
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

TEST(HeartFiberMutex, ExclusiveLock)
{
	HeartFiberSystem::Settings settings;
	settings.threadCount = 2;

	HeartFiberSystem system;
	system.Initialize(settings);

	HeartFiberMutex mutex;
	mutex.LockExclusive(HeartFiberMutex::NeverYield);

	EXPECT_FALSE(mutex.TryLockExclusive()) << "HeartFiberMutex should not support recursive locking";

	system.EnqueueWork([&]() {
		EXPECT_FALSE(mutex.TryLockExclusive()) << "HeartFiberMutex should not support recursive locking";
		return HeartFiberResult::Success;
	});

	system.Shutdown();

	mutex.Unlock();
}

TEST(HeartFiberMutex, LockGuard)
{
	HeartFiberSystem::Settings settings;
	settings.threadCount = 2;

	HeartFiberSystem system;
	system.Initialize(settings);

	HeartFiberMutex mutex;

	{
		HeartLockGuard lock(mutex, HeartFiberMutex::NeverYield);

		EXPECT_FALSE(mutex.TryLockExclusive()) << "HeartFiberMutex should not support recursive locking";

		system.EnqueueWork([&]() {
			EXPECT_FALSE(mutex.TryLockExclusive()) << "HeartFiberMutex should not support recursive locking";
			return HeartFiberResult::Success;
		});

		system.Shutdown();
	}

	EXPECT_TRUE(mutex.TryLockExclusive());
	mutex.Unlock();
}

TEST(Fibers, ShouldNotLeakMemory)
{
	TestTrackingAllocator allocator;

	{
		HeartFiberSystem system(allocator);

		system.Initialize({});

		static int32_t someData = 0;

		system.EnqueueWork([]() {
			EXPECT_EQ(someData, 0);
			someData = 1;
			return HeartFiberResult::Success;
		});

		system.Shutdown();

		EXPECT_EQ(someData, 1);
	}

	EXPECT_EQ(allocator.m_allocatedCount, 0);
	EXPECT_EQ(allocator.m_allocatedSize, 0);
}

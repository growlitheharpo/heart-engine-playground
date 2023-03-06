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
#include <heart/thread/thread.h>

#include <gtest/gtest.h>

#include <atomic>
#include <mutex>

TEST(HeartThread, Threading)
{
	std::atomic_int32_t value(0);
	std::mutex mutex;

	struct UserData
	{
		std::atomic_int32_t* v;
		std::mutex* m;
	} u = {&value, &mutex};

	mutex.lock();
	HeartThread thread([](void* userData) -> void* {
		UserData* d = reinterpret_cast<UserData*>(userData);

		auto& value = *d->v;
		auto& mutex = *d->m;

		// Make sure the main thread has the mutex
		EXPECT_FALSE(mutex.try_lock());
		EXPECT_EQ(value.load(), 0);

		// Tell it we're ready
		++value;

		// Wait for the main thread to release the mutex
		mutex.lock();

		EXPECT_EQ(value.load(), 1);
		++value;
		mutex.unlock();

		return nullptr;
	},
		&u);

	// Wait for the thread to signal 1
	while (value.load() < 1)
	{
	}

	// Test that the thread was initialized
	EXPECT_FALSE(!thread);
	EXPECT_TRUE(!!thread);
	EXPECT_NE(*thread.GetNativeHandle(), nullptr);

	// Give it the mutex
	mutex.unlock();

	// Wait for it to finish
	while (value.load() < 2)
	{
	}

	EXPECT_TRUE(mutex.try_lock());

	// Check that it incremented the value again
	EXPECT_EQ(value.load(), 2);

	// Join it, it should be done.
	thread.Join();
	mutex.unlock();

	EXPECT_TRUE(!thread);
	EXPECT_FALSE(!!thread);
	EXPECT_EQ(*thread.GetNativeHandle(), nullptr);
}

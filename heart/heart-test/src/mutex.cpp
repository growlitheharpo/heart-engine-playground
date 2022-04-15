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
#include <heart/sync/mutex.h>

#include <gtest/gtest.h>

#include <thread>

TEST(HeartMutex, ExclusiveLocking)
{
	HeartMutex m;
	m.LockExclusive();

	EXPECT_FALSE(m.TryLockExclusive()) << "HeartMutex should not support recursive locking";
	EXPECT_FALSE(m.TryLockShared()) << "HeartMutex should not support recursive locking";

	std::thread([&m]() {
		EXPECT_FALSE(m.TryLockExclusive()) << "HeartMutex should already be locked!";
		EXPECT_FALSE(m.TryLockShared()) << "HeartMutex should already be locked!";
	}).join();

	m.Unlock();
}

TEST(HeartMutex, LockGuard)
{
	HeartMutex m;

	{
		HeartLockGuard lock(m);

		EXPECT_FALSE(m.TryLockExclusive()) << "HeartMutex should not support recursive locking";
		EXPECT_FALSE(m.TryLockShared()) << "HeartMutex should not support recursive locking";

		std::thread([&m]() {
			EXPECT_FALSE(m.TryLockExclusive()) << "HeartMutex should already be locked!";
			EXPECT_FALSE(m.TryLockShared()) << "HeartMutex should already be locked!";
		}).join();
	}

	std::thread([&m]() {
		EXPECT_TRUE(m.TryLockExclusive()) << "LockGuard should have released the lock";
		m.Unlock();
	}).join();

	EXPECT_TRUE(m.TryLockExclusive()) << "LockGuard should have released the lock";
	m.Unlock();
}

TEST(HeartMutex, UniqueLock)
{
	HeartMutex m;

	{
		HeartUniqueLock lock(m);

		EXPECT_TRUE(lock.OwnsLock()) << "UniqueLock should hold the lock!";
		EXPECT_FALSE(m.TryLockExclusive()) << "HeartMutex should not support recursive locking";
		EXPECT_FALSE(m.TryLockShared()) << "HeartMutex should not support recursive locking";

		std::thread([&m]() {
			EXPECT_FALSE(m.TryLockExclusive()) << "HeartMutex should already be locked!";
			EXPECT_FALSE(m.TryLockShared()) << "HeartMutex should already be locked!";
		}).join();
	}

	std::thread([&m]() {
		EXPECT_TRUE(m.TryLockExclusive()) << "UniqueLock should have released the lock";
		m.Unlock();
	}).join();

	EXPECT_TRUE(m.TryLockExclusive()) << "UniqueLock should have released the lock";
	m.Unlock();
}

TEST(HeartMutex, SharedLock)
{
	HeartMutex m;

	m.LockShared();

	// Can't be locked exclusively
	std::thread([&m]() {
		EXPECT_FALSE(m.TryLockExclusive());
	}).join();

	// Can be locked shared
	std::thread([&m]() {
		EXPECT_TRUE(m.TryLockShared());
		m.UnlockShared();
	}).join();

	// SharedLock
	std::thread([&m]() {
		{
			HeartSharedLock lock(m);
			EXPECT_TRUE(lock.OwnsLock());
		}
		EXPECT_TRUE(m.TryLockShared());
		m.UnlockShared();
	}).join();

	m.UnlockShared();
	m.LockExclusive();

	// Can't be locked shared
	std::thread([&m]() {
		EXPECT_FALSE(m.TryLockShared());
	}).join();

	m.UnlockShared();
}

#include <heart/jobs/system.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <numeric>
#include <thread>

TEST(HeartJobSystem, OneThread)
{
	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 1;

	HeartJobSystem system;
	system.Initialize(settings);

	std::atomic_bool jobDone = false;
	auto job = system.EnqueueJob([&]() {
		jobDone = true;
		return HeartJobResult::Success;
	});

	// Wait for it to finish
	while (job->status == HeartJobStatus::Pending)
	{
		std::this_thread::yield();
	}

	EXPECT_EQ(job->status.load(), HeartJobStatus::Success);
	EXPECT_EQ(jobDone.load(), true);

	system.Shutdown();
}

TEST(HeartJobSystem, EightThreads)
{
	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 8;

	HeartJobSystem system;
	system.Initialize(settings);

	const int JobCount = 128;
	std::atomic_int doneCount = 0;
	std::vector<HeartJobRef> jobs;

	std::mutex threadIdsMutex;
	std::vector<std::thread::id> threadIds;

	std::generate_n(std::back_inserter(jobs), JobCount, [&]() {
		return system.EnqueueJob([&]() {
			{
				std::lock_guard lock(threadIdsMutex);
				if (std::find(threadIds.begin(), threadIds.end(), std::this_thread::get_id()) == threadIds.end())
				{
					threadIds.push_back(std::this_thread::get_id());
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			doneCount++;

			return HeartJobResult::Success;
		});
	});

	// Wait for them to finish
	while (std::any_of(std::begin(jobs), std::end(jobs), [](const HeartJobRef& j) { return j->status == HeartJobStatus::Pending; }))
	{
		std::this_thread::yield();
	}

	for (auto& job : jobs)
	{
		EXPECT_EQ(job->status.load(), HeartJobStatus::Success);
	}

	// With so many "long-running" jobs added, we would expect that all 7 threads
	// would be engaged. The eighth thread should be locked to Urgent jobs.
	EXPECT_EQ(threadIds.size(), 7);
	EXPECT_EQ(doneCount, JobCount);

	system.Shutdown();
}

TEST(HeartJobSystem, Priorities)
{
	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 1;

	HeartJobSystem system;
	system.Initialize(settings);

	std::atomic_int completeCount = 0;
	int completeOrder[3] = {};

	auto normPriWorker = [&]() {
		int order = ++completeCount;
		completeOrder[0] = order;
		return HeartJobResult::Success;
	};

	auto hiPriWorker = [&]() {
		int order = ++completeCount;
		completeOrder[1] = order;
		return HeartJobResult::Success;
	};

	auto urgentPriWorker = [&]() {
		int order = ++completeCount;
		completeOrder[2] = order;
		return HeartJobResult::Success;
	};

	// Add a job that just sleeps so that we can enqueue our jobs in the "wrong" order without anything being processed
	system.EnqueueJob([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return HeartJobResult::Success;
	});

	auto job1 = system.EnqueueJob(normPriWorker, HeartJobPriority::Normal);
	auto job2 = system.EnqueueJob(hiPriWorker, HeartJobPriority::High);
	auto job3 = system.EnqueueJob(urgentPriWorker, HeartJobPriority::Urgent);

	// Ensure nothing has completed at this point. If this fails, we need to increase our sleep time.
	ASSERT_EQ(completeCount.load(), 0);

	HeartJobRef jobs[] = {job1, job2, job3};
	while (std::any_of(std::begin(jobs), std::end(jobs), [](const HeartJobRef& j) { return j->status == HeartJobStatus::Pending; }))
	{
		std::this_thread::yield();
	}

	// If the jobs were processed in submission order, completeOrder would be { 1, 2, 3 }
	// If the jobs were processed according to priority, completeOrder would be { 3, 2, 1 }
	EXPECT_EQ(completeOrder[0], 3);
	EXPECT_EQ(completeOrder[1], 2);
	EXPECT_EQ(completeOrder[2], 1);

	system.Shutdown();
}

TEST(HeartJobSystem, MoveOnlyJob)
{
	static bool deleted = false;

	struct SomeBigType
	{
		char data[256] = {};

		~SomeBigType()
		{
			deleted = true;
		}

		SomeBigType() = default;
		SomeBigType(const SomeBigType&) = delete;
		SomeBigType& operator=(const SomeBigType&) = delete;
	};

	std::unique_ptr ptr = std::make_unique<SomeBigType>();
	strcat_s(ptr->data, "hello world");

	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 1;

	HeartJobSystem system;
	system.Initialize(settings);

	EXPECT_EQ(deleted, false);

	bool matchingString = false;
	auto job = system.EnqueueJob([p = std::move(ptr), &matchingString]() {
		matchingString = (strcmp(p->data, "hello world") == 0);
		return matchingString ? HeartJobResult::Success : HeartJobResult::Failure;
	});

	while (job->status == HeartJobStatus::Pending)
	{
		std::this_thread::yield();
	}

	EXPECT_EQ(job->status, HeartJobStatus::Success);
	EXPECT_EQ(matchingString, true);

	job = nullptr;
	EXPECT_EQ(deleted, true);

	system.Shutdown();
}

TEST(HeartJobSystem, FailureStatus)
{
	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 1;

	HeartJobSystem system;
	system.Initialize(settings);

	auto job = system.EnqueueJob([]() {
		return HeartJobResult::Failure;
	});

	while (job->status == HeartJobStatus::Pending)
	{
		std::this_thread::yield();
	}

	EXPECT_EQ(job->status, HeartJobStatus::Failure);

	system.Shutdown();
}

TEST(HeartJobSystem, RetryStatus)
{
	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 1;

	HeartJobSystem system;
	system.Initialize(settings);

	int retryCount = 0;
	std::atomic_int counter = 0;

	auto job = system.EnqueueJob([retryCount, &counter]() mutable {
		++retryCount;
		counter = retryCount;

		return retryCount < 5 ? HeartJobResult::Retry : HeartJobResult::Success;
	});

	while (job->status == HeartJobStatus::Pending)
	{
		std::this_thread::yield();
	}

	EXPECT_EQ(job->status, HeartJobStatus::Success);
	EXPECT_EQ(retryCount, 0);
	EXPECT_EQ(counter, 5);

	system.Shutdown();
}

TEST(HeartJobSystem, LargeJob)
{
	const size_t MagicAssumedVtableSize = 8;

	struct BigJobData
	{
		uint8_t data[HeartJobStorage - MagicAssumedVtableSize];
	};

	HeartJobSystem::Settings settings = HeartJobSystem::GetDefaultSettings();
	settings.threadCount = 1;

	HeartJobSystem system;
	system.Initialize(settings);

	BigJobData data;
	std::iota(std::begin(data.data), std::end(data.data), 1);

	auto job = system.EnqueueJob([data]() {
		for (uint8_t i = 0; i < sizeof(data.data); ++i)
		{
			EXPECT_EQ(data.data[i], i + 1);
		}

		return *(std::end(data.data) - 1) == uint8_t(sizeof(data.data)) ? HeartJobResult::Success : HeartJobResult::Failure;
	});

	while (job->status == HeartJobStatus::Pending)
	{
		std::this_thread::yield();
	}

	EXPECT_EQ(job->status.load(), HeartJobStatus::Success);
	system.Shutdown();
}

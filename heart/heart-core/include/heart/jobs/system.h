#pragma once

#include <heart/function.h>
#include <heart/sync/condition_variable.h>
#include <heart/sync/mutex.h>
#include <heart/thread.h>

#include <heart/stl/memory.h>
#include <heart/stl/vector.h>

#include <atomic>
#include <deque>

static constexpr size_t HeartJobStorage = 256;

// Current status of a job. Any side effects of a job should not be
// inspected until the status of the job is no longer Pending.
enum class HeartJobStatus : uint8_t
{
	Pending,
	Failure,
	Success,
};

// Return result from a worker function. Used to update the status.
enum class HeartJobResult : uint8_t
{
	Failure,
	Success,
	Retry,
};

// The priority of a job. Higher priority jobs will be executed
// before lower priority jobs, regardless of submission order.
// When enough threads are available, Urgent jobs will have at
// least one dedicated processing thread.
enum class HeartJobPriority : uint32_t
{
	Normal,
	High,
	Urgent,

	Count,
	Maximum = Urgent,
};

// HeartJobMask is not an enum class to encourage users to create their own masks.
// Masks allow finer-grained work stealing from user code. Masks are ignored
// within the normal workings of the job system.
enum HeartJobMask : uint32_t
{
	HeartJobMaskNone = 0,
	HeartJobMaskDefault = 1,
	HeartJobMaskAll = ~HeartJobMaskNone,
};

// A pending job within the job system. Can only be constructed by the job system.
struct HeartJob
{
private:
	friend class HeartJobSystem;

	// Because we use make_shared, our constructor must be public.
	// We use this secret to ensure only the job system can construct a job.
	struct ConstructorSecretType
	{
	};
	static constexpr ConstructorSecretType ConstructorSecret = {};

	uint32_t mask = HeartJobMaskDefault;
	HeartFunction<HeartJobResult(), HeartJobStorage> worker = {};

public:
	template <typename F>
	HeartJob(ConstructorSecretType, F&& f, uint32_t m) :
		worker(hrt::forward<F>(f)),
		mask(m),
		status(HeartJobStatus::Pending)
	{
	}

	// The status of this job. Side effects of the job should not
	// be inspected until status does not equal Pending.
	std::atomic<HeartJobStatus> status = HeartJobStatus::Pending;
};

using HeartJobRef = hrt::shared_ptr<HeartJob>;

class HeartJobSystem final
{
public:
	struct Settings
	{
		// How many theads the job system will use.
		uint8_t threadCount;

		// The priority of the threads created by the job system.
		HeartThread::Priority threadPriority;
	};

	// Constructs reasonable default settings based on available hardware concurrency.
	static Settings GetDefaultSettings();

private:
	HeartMutex m_queueMutex;
	HeartConditionVariable m_conditionVar;

	std::deque<HeartJobRef> m_queues[(uint32_t)HeartJobPriority::Count];

	hrt::vector<HeartThread> m_workerThreads;
	std::atomic_bool m_exit = false;

	static void* ThreadEntryPoint(void* p);
	void ThreadWorker(HeartJobPriority lowestPriority);

	bool TryAcquireOneJob(HeartJobRef& outJob, HeartJobPriority& outPri, HeartJobPriority lowestPri, uint32_t mask = HeartJobMaskAll);
	void ProcessOneJob(HeartJobRef job, HeartJobPriority priority);

public:
	HeartJobSystem() = default;
	~HeartJobSystem();

	// Prepare the job system.
	void Initialize(Settings s = GetDefaultSettings());

	// Flush all pending jobs.
	// NOTE: This does *NOT* wait for every job to finish executing, only
	// for there to be no pending jobs in the queue.
	void Flush();

	// Shutdown the job system. Waits for all pending work to complete.
	void Shutdown();

	// Insert a new job into the job system. F can be any invokable type
	// as long as it fits within HeartJobStorage and returns a HeartJobResult.
	template <typename F>
	HeartJobRef EnqueueJob(F&& f, HeartJobPriority pri = HeartJobPriority::Normal, uint32_t mask = HeartJobMaskDefault)
	{
		HeartJobRef newJob = hrt::make_shared<HeartJob>(HeartJob::ConstructorSecret, hrt::forward<F>(f), mask);

		{
			HeartLockGuard lock(m_queueMutex);
			m_queues[int(pri)].push_back(newJob);
		}

		m_conditionVar.NotifyOne();
		return newJob;
	}

	// Attempt to steal work from the job system instead of waiting for it
	// to execute on a job thread. The user can specify a mask and priority
	// to limit what will be stolen.
	// Returns whether or not a job was stolen.
	bool TryStealJobWork(HeartJobPriority lowestPriority = HeartJobPriority::Normal, uint32_t mask = HeartJobMaskAll);
};

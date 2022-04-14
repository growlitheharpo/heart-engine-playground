#pragma once

#include <heart/allocator.h>
#include <heart/function/embedded_function.h>
#include <heart/memory/intrusive_list.h>
#include <heart/memory/intrusive_ptr.h>
#include <heart/memory/vector.h>
#include <heart/sync/condition_variable.h>
#include <heart/sync/mutex.h>
#include <heart/thread.h>

#include <atomic>

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

	mutable std::atomic<uint32_t> useCount = 0;

	// The mask for executing this job
	uint32_t mask = HeartJobMaskDefault;

	// Our intrusive link into the job queue
	HeartIntrusiveListLink link;

	// The allocator from which we were created
	HeartBaseAllocator& allocator;

	// Our actual job execution function
	HeartEmbeddedFunction<HeartJobResult(), HeartJobStorage> worker = {};

public:
	template <typename F>
	HeartJob(ConstructorSecretType, HeartBaseAllocator& a, F&& f, uint32_t m) :
		worker(hrt::forward<F>(f)),
		allocator(a),
		mask(m),
		status(HeartJobStatus::Pending)
	{
	}

	// The status of this job. Side effects of the job should not
	// be inspected until status does not equal Pending.
	std::atomic<HeartJobStatus> status = HeartJobStatus::Pending;

	void IncrementRef() const
	{
		++useCount;
	}

	void DecrementRef() const
	{
		if (--useCount == 0)
		{
			allocator.DestroyAndFree(this);
		}
	}

	uint32_t GetRefCount()
	{
		return useCount;
	}
};

using HeartJobRef = HeartIntrusivePtr<HeartJob>;

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
	typedef HeartIntrusiveList<HeartJob, &HeartJob::link> JobQueue;

	// Our allocator. All allocations from the job system must come out of this allocator.
	HeartBaseAllocator& m_allocator;

	// Synchronization for the job queue and worker threads
	HeartMutex m_queueMutex;
	HeartConditionVariable m_conditionVar;

	// The queues. "Zero" allocation intrusive lists which link together our job nodes.
	JobQueue m_queues[(uint32_t)HeartJobPriority::Count];

	// The vector of our workers
	heart_priv::HeartVector<HeartThread> m_workerThreads;

	// Exit flag
	std::atomic_bool m_exit = false;

private:
	// Thread entry point and workers for the job system threads
	static void* ThreadEntryPoint(void* p);
	void ThreadWorker(HeartJobPriority lowestPriority);

	// Insert a new job into the queue. Do NOT hold the mutex when calling this.
	void InsertJobIntoQueue(HeartJob* rawJob, HeartJobPriority pri);

	// Remove a job from the queue. You MUST hold the mutex when calling this.
	HeartJobRef RemoveJobFromQueue(JobQueue& queue, JobQueue::iterator iterator);

	// Attempt to pop a job from the queue with the given mask and priority limit.
	// Returns true if a job was successfully popped.
	bool TryAcquireOneJob(HeartJobRef& outJob, HeartJobPriority& outPri, HeartJobPriority lowestPri, uint32_t mask = HeartJobMaskAll);

	// Executes the provided job. If the job returns Retry, requeues it with the provided priority.
	void ProcessOneJob(HeartJobRef job, HeartJobPriority priority);

public:
	HeartJobSystem(HeartBaseAllocator& allocator = GetHeartDefaultAllocator());
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
		HeartJobRef newJob = m_allocator.AllocateAndConstruct<HeartJob>(HeartJob::ConstructorSecret, m_allocator, hrt::forward<F>(f), mask);

		InsertJobIntoQueue(newJob.Get(), pri);
		m_conditionVar.NotifyOne();

		return newJob;
	}

	// Attempt to steal work from the job system instead of waiting for it
	// to execute on a job thread. The user can specify a mask and priority
	// to limit what will be stolen.
	// Returns whether or not a job was stolen.
	bool TryStealJobWork(HeartJobPriority lowestPriority = HeartJobPriority::Normal, uint32_t mask = HeartJobMaskAll);
};

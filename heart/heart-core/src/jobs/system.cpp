#include <heart/jobs/system.h>

#include <heart/sleep.h>

#include <algorithm>
#include <thread>

struct ThreadUserData
{
	HeartJobSystem* system = nullptr;
	HeartJobPriority lowestPriority = HeartJobPriority::Normal;
	std::atomic_bool started = false;
};

HeartJobSystem::Settings HeartJobSystem::GetDefaultSettings()
{
	Settings defaultSettings;

	// Start with the actual physical concurrency
	uint32_t coreCount = std::thread::hardware_concurrency();

	// Assume the game is going to have some threads of their own, eg audio, file io, and the main thread
	const uint32_t ReservedCount = 4;

	// No matter what, we'll need at least 1 job thread
	const uint32_t MinimumCount = 1;

	// We don't want to create an insane number of threads though - the user might
	// be creating thread_local resources and probably isn't expecting hundreds of them on a Threadripper.
	const uint32_t MaximumCount = 16;

	defaultSettings.threadCount = uint8_t(std::clamp(coreCount - ReservedCount, MinimumCount, MaximumCount));
	defaultSettings.threadPriority = HeartThread::Priority::High;
	return defaultSettings;
}

void* HeartJobSystem::ThreadEntryPoint(void* p)
{
	ThreadUserData* userData = (ThreadUserData*)p;

	HeartJobSystem* s = userData->system;
	HeartJobPriority pri = userData->lowestPriority;
	userData->started.store(true, std::memory_order_relaxed);

	s->ThreadWorker(pri);
	return nullptr;
}

HeartJobSystem::~HeartJobSystem()
{
	if (!m_exit)
	{
		Shutdown();
	}
}

void HeartJobSystem::Initialize(Settings s)
{
	int threadCount = s.threadCount < 1 ? 1 : s.threadCount;
	int urgentThreadCount = threadCount > 1 ? 1 : 0;

	for (int i = 0; i < threadCount; ++i)
	{
		ThreadUserData ud;
		ud.system = this;
		ud.lowestPriority = i < urgentThreadCount ? HeartJobPriority::Urgent : HeartJobPriority::Normal;

		auto& t = m_workerThreads.emplace_back(&ThreadEntryPoint, &ud, s.threadPriority);
		t.SetName("HeartJobSystem Thread");

		while (!ud.started.load(std::memory_order_relaxed))
		{
			HeartYield();
		}
	}
}

void HeartJobSystem::Flush()
{
	bool any = true;

	while (any)
	{
		any = false;

		{
			HeartLockGuard lock(m_queueMutex);
			for (auto& q : m_queues)
			{
				if (!q.empty())
				{
					any = true;
					break;
				}
			}
		}

		if (any)
		{
			HeartYield();
		}
	}
}

void HeartJobSystem::Shutdown()
{
	Flush();

	m_exit = true;
	m_conditionVar.NotifyAll();

	for (auto& t : m_workerThreads)
		t.Join();
	m_workerThreads.clear();
}

void HeartJobSystem::ThreadWorker(HeartJobPriority lowestPriority)
{
	HeartJobRef currentJob = nullptr;
	HeartJobPriority currentPriority = HeartJobPriority::Count;

	while (m_exit.load(std::memory_order_acquire) == false)
	{
		{
			HeartLockGuard lock(m_queueMutex);

			while (m_exit.load(std::memory_order_acquire) == false && !TryAcquireOneJob(currentJob, currentPriority, lowestPriority))
			{
				m_conditionVar.TryWaitFor(m_queueMutex, 10);
			}
		}

		if (currentJob)
		{
			ProcessOneJob(currentJob, currentPriority);
			currentJob = nullptr;
		}
	}
}

bool HeartJobSystem::TryAcquireOneJob(HeartJobRef& outJob, HeartJobPriority& outPri, HeartJobPriority lowestPri, uint32_t mask)
{
	// PRE-CONDITION: The caller *must* be holding the queue mutex!

	outJob = nullptr;

	for (int i = int(HeartJobPriority::Maximum); i >= int(lowestPri); --i)
	{
		auto& q = m_queues[i];

		for (auto iter = q.begin(); iter != q.end(); ++iter)
		{
			HeartJobRef& job = *iter;
			if (job->mask & mask)
			{
				outJob = job;
				outPri = HeartJobPriority(i);
				q.erase(iter);
				return true;
			}
		}
	}

	return false;
}

void HeartJobSystem::ProcessOneJob(HeartJobRef job, HeartJobPriority priority)
{
	auto result = job->worker();
	if (result == HeartJobResult::Success)
	{
		job->status = HeartJobStatus::Success;
		job->worker.Clear();
	}
	else if (result == HeartJobResult::Failure)
	{
		job->status = HeartJobStatus::Failure;
		job->worker.Clear();
	}
	else if (result == HeartJobResult::Retry)
	{
		{
			HeartLockGuard lock(m_queueMutex);
			m_queues[int(priority)].push_back(job);
		}

		m_conditionVar.NotifyOne();
	}
}

bool HeartJobSystem::TryStealJobWork(HeartJobPriority lowestPriority, uint32_t mask)
{
	HeartJobRef job;
	HeartJobPriority pri;

	{
		HeartLockGuard lock(m_queueMutex);
		if (!TryAcquireOneJob(job, pri, lowestPriority, mask))
			return false;
	}

	ProcessOneJob(job, pri);
	return true;
}

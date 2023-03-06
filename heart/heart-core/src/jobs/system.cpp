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
#include "heart/jobs/system.h"

#include "heart/sleep.h"
#include "heart/thread/bootstrap.h"

#include <algorithm>
#include <thread>

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

HeartJobSystem::HeartJobSystem(HeartBaseAllocator& allocator) :
	m_allocator(allocator),
	m_workerThreads(allocator)
{
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

	m_workerThreads.Reserve(threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		auto lowestPriority = i < urgentThreadCount ? HeartJobPriority::Urgent : HeartJobPriority::Normal;
		HeartThread& thread = m_workerThreads.EmplaceBack(HeartThreadMemberBootstrap(this, &HeartJobSystem::ThreadWorker, lowestPriority));
		thread.SetName("HeartJobSystem Thread");
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
				if (!q.IsEmpty())
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

	for (HeartThread& workerThread : m_workerThreads)
	{
		workerThread.Join();
	}

	m_workerThreads.Clear();
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

void HeartJobSystem::InsertJobIntoQueue(HeartJob* rawJob, HeartJobPriority pri)
{
	HeartLockGuard lock(m_queueMutex);
	m_queues[int(pri)].PushBack(rawJob);

	// We manually increment the ref count here. This is the "queue ref"
	// since the queue itself holds raw pointers, not intrusive pointers
	rawJob->IncrementRef();
}

HeartJobRef HeartJobSystem::RemoveJobFromQueue(JobQueue& queue, JobQueue::iterator iterator)
{
	// PRE-CONDITION: The caller *must* be holding the queue mutex!

	HeartJob* rawPtr = iterator.operator->();
	HeartJobRef strongRef = rawPtr;

	// We manually decrement the ref count here. This is the "queue ref"
	// since the queue itself holds raw pointers, not intrusive pointers.
	// This must come *after* we assign strongRef above, or else it could
	// cause the job to be deleted before we return it.
	rawPtr->DecrementRef();

	queue.Remove(iterator);

	return strongRef;
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
			HeartJob& job = *iter;
			if (job.mask & mask)
			{
				outPri = HeartJobPriority(i);
				outJob = RemoveJobFromQueue(q, iter);
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
		job->worker.Clear();
		job->status = HeartJobStatus::Success;
	}
	else if (result == HeartJobResult::Failure)
	{
		job->worker.Clear();
		job->status = HeartJobStatus::Failure;
	}
	else if (result == HeartJobResult::Retry)
	{
		InsertJobIntoQueue(job.Get(), priority);
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

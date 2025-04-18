#include "Threading/Jobs/JobRunnerThread.h"
#include "Threading/Jobs/Job.h"
#include "Threading/Jobs/JobManager.h"

#include <Common/Platform/Windows.h>
#include <Common/Math/Min.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Floor.h>
#include <Common/Math/Random.h>

#include <algorithm>

namespace ngine::Threading
{
	JobRunnerThread::JobRunnerThread(JobManager& manager)
		: m_manager(manager)
	{
	}

	JobRunnerThread::~JobRunnerThread()
	{
		StopThread();

		for (Job& job : GetThreadSafeQueueView())
		{
			[[maybe_unused]] const bool cleared = job.m_stateFlags.FetchAnd(~Job::StateFlags::Queued).IsSet(Job::StateFlags::Queued);
			Assert(cleared);
		}

		for (Job& job : GetThreadSafeExclusiveQueueView())
		{
			[[maybe_unused]] const bool cleared = job.m_stateFlags.FetchAnd(~Job::StateFlags::Queued).IsSet(Job::StateFlags::Queued);
			Assert(cleared);
		}

		for (Job& job : m_exclusiveQueue)
		{
			[[maybe_unused]] const bool cleared = job.m_stateFlags.FetchAnd(~Job::StateFlags::Queued).IsSet(Job::StateFlags::Queued);
			Assert(cleared);
		}

		for (Job& job : m_queue)
		{
			[[maybe_unused]] const bool cleared = job.m_stateFlags.FetchAnd(~Job::StateFlags::Queued).IsSet(Job::StateFlags::Queued);
			Assert(cleared);
		}

		if (m_pNextJob != nullptr)
		{
			[[maybe_unused]] const bool cleared = m_pNextJob->m_stateFlags.FetchAnd(~Job::StateFlags::Queued).IsSet(Job::StateFlags::Queued);
			Assert(cleared);
		}
	}

	void JobRunnerThread::StartThread(const ConstNativeStringView name, const ThreadIndexType threadIndex)
	{
		m_threadIndex = threadIndex;

		m_flags |= Flags::IsStartingThread;
		BaseType::Start(name);
		m_threadId = BaseType::GetThreadId();
		m_flags &= ~Flags::IsStartingThread;

		// BaseType::SetAffinityMask(1ull << threadIndex);
	}

	void JobRunnerThread::StopThread()
	{
		m_flags |= Flags::IsQuitting;
		m_idleConditionVariable.NotifyOne();
		Join();
	}

	void JobRunnerThread::InitializeMainThread()
	{
		m_threadIndex = 0;
		m_threadId = ThreadId::GetCurrent();

		// SetAffinityMask(m_threadId, 1ull << 0);
		GetCurrent() = this;

		BaseType::InitializeExternallyCreatedFromThread();
	}

	void JobRunnerThread::Run()
	{
		while (m_flags.IsSet(Flags::IsStartingThread))
			;

		GetCurrent() = this;

		SetDefaultPriority();

		// Initialize random generator for this thread
		{
			[[maybe_unused]] const auto value = Math::Random(0u, 1u);
		}

		while (!m_flags.IsSet(Flags::IsQuitting))
		{
			// Assert(!m_manager.IsThreadIdle(m_threadIndex));
			Assert(!m_flags.IsSet(Flags::IsIdle));

#if PLATFORM_APPLE
			@autoreleasepool
			{
				Tick();
			}
#else
			Tick();
#endif
		}
	}

	void JobRunnerThread::Tick()
	{
		if (!DoRunNextJob())
		{
			// No work is available to us right now
			if (!m_flags.AreAnySet(Flags::HasWork))
			{
				{
					Threading::UniqueLock idleLock(m_threadSafeQueueMutex);
					if (m_flags.AreNoneSet(Flags::HasAnyJobsInThreadSafeQueues))
					{
						m_manager.MarkThreadAsIdle(m_threadIndex);
						[[maybe_unused]] EnumFlags<Flags> previousFlags = m_flags.FetchOr(Flags::IsIdle);
						Assert(!previousFlags.IsSet(Flags::IsIdle));
						Assert(previousFlags.AreNoneSet(Flags::HasWork));
						// do
						{
							m_idleConditionVariable.Wait(idleLock);
						} // while (!m_flags.AreAnySet(Flags::HasAnyJobsInThreadSafeQueues));
						Assert(idleLock.IsLocked());

						m_manager.ClearThreadIdleFlag(m_threadIndex);
						previousFlags = m_flags.FetchAnd(~Flags::IsIdle);
						Assert(previousFlags.IsSet(Flags::IsIdle));
					}
				}
			}
		}
	}

	void JobRunnerThread::UpdateIdleState()
	{
		Threading::UniqueLock idleLock(m_threadSafeQueueMutex);
		if (m_flags.AreAnySet(Flags::HasWork))
		{
			m_manager.ClearThreadIdleFlag(m_threadIndex);
		}
		else
		{
			m_manager.MarkThreadAsIdle(m_threadIndex);
		}
	}

	void JobRunnerThread::Wake()
	{
		Threading::UniqueLock idleLock(m_threadSafeQueueMutex);
		m_idleConditionVariable.NotifyOne();
	}

	bool JobRunnerThread::DoRunNextJob()
	{
		const EnumFlags<Flags> flags = reinterpret_cast<const EnumFlags<Flags>&>(m_flags);
		if (flags.IsSet(Flags::HasNextJob))
		{
			RunNextJob();

#if USE_PRIORITY_QUEUE
			m_queue.ShiftToOrigin();
#endif
			return true;
		}
		else if (flags.AreAnySet(Flags::HasAnyJobsInQueues))
		{
			MoveThreadSafeQueueElements();
			UpdateNextJob();
			if (m_pNextJob != nullptr)
			{
				RunNextJob();
			}
			return true;
		}

		return false;
	}

	bool JobRunnerThread::IsExecutingOnThread() const
	{
		return Threading::ThreadId::GetCurrent() == m_threadId;
	}

	bool JobRunnerThread::TrySetNextJob(Job& job)
	{
		Assert((job.GetAllowedJobRunnerMask() & (1ull << m_threadIndex)) != 0);
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(IsExecutingOnThread());

		Job* pPreviousNextJob = m_pNextJob;
		Assert(&job != pPreviousNextJob);

		if (pPreviousNextJob == nullptr)
		{
			m_pNextJob = &job;
			m_flags |= Flags::HasNextJob;
			UpdatePriorityForJob(job);
			return true;
		}

		if (job.GetPriority() < pPreviousNextJob->GetPriority())
		{
			m_pNextJob = &job;
			UpdatePriorityForJob(job);
			Assert(m_flags.IsSet(Flags::HasNextJob));

			const bool isExclusiveToRunner = pPreviousNextJob->GetAllowedJobRunnerMask() == (1ull << m_threadIndex);
			if (isExclusiveToRunner)
			{
				QueueExclusiveJobImmediate(*pPreviousNextJob);
			}
			else
			{
				QueueJobImmediate(*pPreviousNextJob);
			}

			return true;
		}

		return false;
	}

	bool JobRunnerThread::QueueJobFromThread(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.IsQueued());
		Assert(IsExecutingOnThread());

		const EnumFlags<Flags> flags = m_flags.GetFlags();
		const bool canRunJob = (job.IsLowPriorityPerformanceJob() & flags.IsSet(Flags::CanRunLowPriorityPerformanceJobs)) |
		                       (job.IsHighPriorityPerformanceJob() & flags.IsSet(Flags::CanRunHighPriorityPerformanceJobs)) |
		                       (job.IsEfficiencyJob() & flags.IsSet(Flags::CanRunEfficiencyJobs));
		if (canRunJob)
		{
			if (!TrySetNextJob(job))
			{
				if (!ShareJobWithFirstIdleThread(job))
				{
					const bool isExclusiveToRunner = job.GetAllowedJobRunnerMask() == (1ull << m_threadIndex);
					if (isExclusiveToRunner)
					{
						QueueExclusiveJobImmediate(job);
					}
					else
					{
						QueueJobImmediate(job);
					}
				}
			}
		}
		return canRunJob;
	}

	bool JobRunnerThread::QueueJobFromAnyThread(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.IsQueued());

		const EnumFlags<Flags> flags = m_flags.GetFlags();
		const bool canRunJob = (job.IsLowPriorityPerformanceJob() & flags.IsSet(Flags::CanRunLowPriorityPerformanceJobs)) |
		                       (job.IsHighPriorityPerformanceJob() & flags.IsSet(Flags::CanRunHighPriorityPerformanceJobs)) |
		                       (job.IsEfficiencyJob() & flags.IsSet(Flags::CanRunEfficiencyJobs));
		if (canRunJob)
		{
			QueueJobImmediateThreadSafe(job);
		}
		return canRunJob;
	}

	void JobRunnerThread::QueueJobImmediate(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.GetAllowedJobRunnerMask() != (1ull << m_threadIndex), "Job must not be exclusive to thread");
		Assert(job.IsQueued());
		Assert(!m_queue.Contains(job));
		Assert(!m_exclusiveQueue.Contains(job));

		Assert(IsExecutingOnThread() | m_flags.IsSet(Flags::IsIdle));

		ReferenceWrapper<Job>* __restrict pUpperBoundIt = std::upper_bound(
			m_queue.begin().Get(),
			m_queue.end().Get(),
			job.GetPriority(),
			[](const JobPriority priority, const Job& job) -> bool
			{
				return priority > job.GetPriority();
			}
		);

		Assert(m_pNextJob == nullptr || job.GetPriority() >= m_pNextJob->GetPriority());
		m_queue.Emplace(pUpperBoundIt, Memory::Uninitialized, job);
		m_flags |= Flags::HasJobsInLocalQueue;
	}

	void JobRunnerThread::QueueExclusiveJobFromThread(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.IsQueued());

		Assert(IsExecutingOnThread());

		job.SetAllowedJobRunnerMask(1ull << m_threadIndex);

		if (!TrySetNextJob(job))
		{
			QueueExclusiveJobImmediate(job);
		}
	}

	void JobRunnerThread::QueueExclusiveJobFromAnyThread(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.IsQueued());

		job.SetAllowedJobRunnerMask(1ull << m_threadIndex);

		QueueJobImmediateThreadSafe(job);
	}

	void JobRunnerThread::QueueExclusiveJobImmediate(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.IsQueued());
		Assert(!m_queue.Contains(job));
		Assert(!m_exclusiveQueue.Contains(job));

		Assert(IsExecutingOnThread() | m_flags.IsSet(Flags::IsIdle));
		ReferenceWrapper<Job>* __restrict pUpperBoundIt = std::upper_bound(
			m_exclusiveQueue.begin().Get(),
			m_exclusiveQueue.end().Get(),
			job.GetPriority(),
			[](const JobPriority priority, const Job& existingJob) -> bool
			{
				return priority > existingJob.GetPriority();
			}
		);

		Assert(m_pNextJob == nullptr || job.GetPriority() >= m_pNextJob->GetPriority());
		m_exclusiveQueue.Emplace(pUpperBoundIt, Memory::Uninitialized, job);
		m_flags |= Flags::HasJobsInLocalExclusiveQueue;
	}

	void JobRunnerThread::QueueJobImmediateThreadSafe(Job& __restrict job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		Assert(job.IsQueued());

		Threading::UniqueLock lock(m_threadSafeQueueMutex);
		m_threadSafeQueueSize++;

		const bool isExclusiveToRunner = job.GetAllowedJobRunnerMask() == (1ull << m_threadIndex);
		if (isExclusiveToRunner)
		{
			m_threadSafeExclusiveQueue.EmplaceBack(job);
			m_flags |= Flags::HasJobsInThreadSafeExclusiveQueue;
		}
		else
		{
			m_threadSafeQueue.EmplaceBack(job);
			m_flags |= Flags::HasJobsInThreadSafeQueue;
		}

		if (m_flags.IsSet(Flags::IsIdle))
		{
			lock.Unlock();
			m_idleConditionVariable.NotifyOne();
		}
	}

	/* static */ void JobRunnerThread::QueueOnIdealRunner(JobManager& manager, Job& job)
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		JobRunnerMask allowedRunnerMask = job.GetAllowedJobRunnerMask();
		if (allowedRunnerMask == Math::NumericLimits<JobRunnerMask>::Max)
		{
			// Job has not opted in for explicit runner selection
			// Calculate based on job priority
			allowedRunnerMask = manager.GetPreferredRunnerMask(job.GetPriority());
		}
		Assert(allowedRunnerMask != 0);
		if (Optional<uint8> idleThreadIndex = manager.StealFirstIdleThread(allowedRunnerMask))
		{
			JobRunnerThread& idleThread = manager.GetJobThreads()[idleThreadIndex.Get()];

			[[maybe_unused]] const bool wasQueued = idleThread.QueueJobFromAnyThread(job);
			Assert(wasQueued);
		}
		else
		{
			Optional<JobRunnerThread*> bestRunner;
			uint16 bestRunnerJobCount = Math::NumericLimits<uint16>::Max;

			for (const uint8 threadIndex : Memory::GetSetBitsIterator(allowedRunnerMask))
			{
				JobRunnerThread& runner = manager.GetJobThreads()[threadIndex];
				const uint16 runnerJobCount = runner.GetQueuedJobCount();
				if (runnerJobCount <= bestRunnerJobCount)
				{
					const bool canRunJob = (job.IsLowPriorityPerformanceJob() & runner.CanRunLowPriorityPerformanceJobs()) |
					                       (job.IsHighPriorityPerformanceJob() && runner.CanRunHighPriorityPerformanceJobs()) |
					                       (job.IsEfficiencyJob() && runner.CanRunEfficiencyJobs());
					if (canRunJob)
					{
						bestRunnerJobCount = runnerJobCount;
						bestRunner = runner;
					}
				}
			}

			if (!bestRunner->QueueJobFromAnyThread(job))
			{
				JobRunnerThread::QueueOnIdealRunner(manager, job);
			}
		}
	}

	void JobRunnerThread::QueueJobsFromThread(JobView jobs)
	{
		Assert(IsExecutingOnThread());

		for (Job& job : jobs)
		{
			job.FlagQueued();
		}

		if (jobs.HasElements())
		{
			ShareJobsWithIdleThreads(
				jobs,
				m_exclusiveQueue.GetSize() + m_queue.GetSize() + uint16(m_pNextJob != nullptr) + GetThreadSafeQueueSize()
			);

			// 'jobs' now contains remaining not shared jobs
		}

		for (Job& job : jobs)
		{
			if (!QueueJobFromThread(job))
			{
				JobRunnerThread::QueueOnIdealRunner(GetJobManager(), job);
			}
		}
	}

	void JobRunnerThread::TryQueueJobsFromThread(JobView jobs)
	{
		Assert(IsExecutingOnThread());
		for (Job& job : jobs)
		{
			job.TryQueue(*this);
		}
	}

	void JobRunnerThread::ShareJobsWithIdleThreads(JobView& jobs, const uint16 totalExistingJobCount) const
	{
		{
			const uint16 totalJobCount = totalExistingJobCount + jobs.GetSize();
			if ((totalJobCount <= 1u) | jobs.IsEmpty())
			{
				// Nothing to share
				return;
			}
		}

		DynamicBitset<uint16>& __restrict triviallyShareableJobs = m_triviallyShareableJobs;
		triviallyShareableJobs.ClearAll();
		triviallyShareableJobs.Reserve(jobs.GetSize());

		triviallyShareableJobs.SetAll(Math::Range<uint32>::Make(0, jobs.GetSize()));
		Assert(triviallyShareableJobs.GetNumberOfSetBits() == jobs.GetSize());
		for (ReferenceWrapper<Job>& job : jobs)
		{
			if (job->GetAllowedJobRunnerMask() != Math::NumericLimits<JobRunnerMask>::Max)
			{
				const uint16 index = jobs.GetIteratorIndex(Memory::GetAddressOf(job));
				triviallyShareableJobs.Clear(index);
			}
		}

		// TODO: Investigate sharing for jobs that have a custom GetAllowedJobRunnerMask
		// At the moment we keep those on their initial runners.

		const JobRunnerMask allowedRunnerMask =
			(m_manager.GetPerformanceLowPriorityThreadMask() * m_flags.IsSet(Flags::CanRunLowPriorityPerformanceJobs)) |
			(m_manager.GetPerformanceHighPriorityThreadMask() * m_flags.IsSet(Flags::CanRunHighPriorityPerformanceJobs)) |
			(m_manager.GetEfficiencyThreadMask() * m_flags.IsSet(Flags::CanRunEfficiencyJobs));

		const uint16 triviallyShareableJobCount = (uint16)triviallyShareableJobs.GetNumberOfSetBits();
		const uint16 maximumSharedJobCount = triviallyShareableJobCount - uint16(totalExistingJobCount == 0);

		Assert(allowedRunnerMask != 0);
		const JobRunnerMask idleThreadMask =
			m_manager.StealIdleThreads((uint8)Math::Min(maximumSharedJobCount, (uint16)m_manager.GetJobThreads().GetSize()), allowedRunnerMask);
		if (idleThreadMask != 0)
		{
			const uint8 numIdleThreads = (uint8)Memory::GetNumberOfSetBits(idleThreadMask);
			Assert(numIdleThreads <= triviallyShareableJobCount);

			const uint16 numJobsPerThread = (uint16)Math::Floor((float)triviallyShareableJobCount / (float)(numIdleThreads + 1));
			uint16 numKeptJobs = numJobsPerThread + ((uint16)triviallyShareableJobCount - numJobsPerThread * (numIdleThreads + 1));
			numKeptJobs -= Math::Min(numKeptJobs, totalExistingJobCount);

			{
				// JobView remainingJobs = jobs;
				uint16 nextKeptJobIndex = 0;

				JobRunnerMask unusedThreadMask = idleThreadMask;

				while (triviallyShareableJobs.AreAnySet())
				{
					if (nextKeptJobIndex < numKeptJobs)
					{
						const uint16 firstSetIndex = (uint16)*triviallyShareableJobs.GetFirstSetIndex();
						jobs[nextKeptJobIndex++] = jobs[firstSetIndex];
						triviallyShareableJobs.Clear(firstSetIndex);
					}
					if (triviallyShareableJobs.AreAnySet())
					{
						for (const uint8 idleThreadIndex : Memory::GetSetBitsIterator(idleThreadMask))
						{
							JobRunnerThread& idleThread = m_manager.GetJobThreads()[idleThreadIndex];
							const uint16 firstSetIndex = (uint16)*triviallyShareableJobs.GetFirstSetIndex();

							[[maybe_unused]] const bool wasQueued = idleThread.QueueJobFromAnyThread(jobs[firstSetIndex]);
							Assert(wasQueued);
							unusedThreadMask &= ~(1 << idleThreadIndex);

							triviallyShareableJobs.Clear(firstSetIndex);
							if (!triviallyShareableJobs.AreAnySet())
							{
								break;
							}
						}
					}
				}

				jobs = jobs.GetSubView(0u, nextKeptJobIndex);

				for (const uint8 idleThreadIndex : Memory::GetSetBitsIterator(unusedThreadMask))
				{
					JobRunnerThread& idleThread = m_manager.GetJobThreads()[idleThreadIndex];
					idleThread.UpdateIdleState();
				}
			}
		}
	}

	bool JobRunnerThread::ShareJobWithFirstIdleThread(Job& job) const
	{
		Assert(Memory::GetAddressOf(job) != nullptr);
		JobRunnerMask allowedRunnerMask = job.GetAllowedJobRunnerMask();
		if (allowedRunnerMask == Math::NumericLimits<JobRunnerMask>::Max)
		{
			// Job has not opted in for explicit runner selection
			// Calculate based on job priority
			allowedRunnerMask = m_manager.GetPreferredRunnerMask(job.GetPriority());
		}
		Assert(allowedRunnerMask != 0);
		if (Optional<uint8> idleThreadIndex = m_manager.StealFirstIdleThread(allowedRunnerMask))
		{
			JobRunnerThread& idleThread = m_manager.GetJobThreads()[idleThreadIndex.Get()];

			[[maybe_unused]] const bool wasQueued = idleThread.QueueJobFromAnyThread(job);
			Assert(wasQueued);
			return true;
		}

		return false;
	}

	void JobRunnerThread::GiveWorkToOtherThreads()
	{
		Assert(IsExecutingOnThread());
		JobView jobs = m_queue.GetView();
		ShareJobsWithIdleThreads(jobs, uint16(m_pNextJob != nullptr) + GetThreadSafeQueueSize() + m_exclusiveQueue.GetSize());
		if (m_queue.GetSize() != jobs.GetSize())
		{
			// jobs now contains the jobs we didn't share
			m_queue.Remove(m_queue.GetSubView(jobs.GetSize(), m_queue.GetSize() - jobs.GetSize()));

			m_flags &= ~(Flags::HasJobsInLocalQueue * m_queue.IsEmpty());
			Assert(m_queue.GetSize() == jobs.GetSize());
		}
	}

	void JobRunnerThread::RunNextJob()
	{
		MoveThreadSafeQueueElements();

		Assert(IsExecutingOnThread());
		Expect(m_pNextJob != nullptr);
		Assert(!m_flags.IsSet(Flags::IsIdle));

		Job* pJob = m_pNextJob;
		m_pNextJob = nullptr;
		m_flags.Clear(Flags::HasNextJob);

		// Before executing, give work to other idle threads if possible
		{
			JobView jobs = m_queue.GetView();
			// Indicate that we have a job (the one we are about to execute)
			ShareJobsWithIdleThreads(jobs, 1 + GetThreadSafeQueueSize() + m_exclusiveQueue.GetSize());
			if (m_queue.GetSize() != jobs.GetSize())
			{
				// jobs now contains the jobs we didn't share
				m_queue.Remove(m_queue.GetSubView(jobs.GetSize(), m_queue.GetSize() - jobs.GetSize()));

				m_flags &= ~(Flags::HasJobsInLocalQueue * m_queue.IsEmpty());
				Assert(m_queue.GetSize() == jobs.GetSize());
			}
		}

		{
			EnumFlags<Job::StateFlags> previousStateFlags = pJob->m_stateFlags.GetFlags();
			EnumFlags<Job::StateFlags> newStateFlags;
			do
			{
				newStateFlags = previousStateFlags;

				Assert(newStateFlags.IsSet(Job::StateFlags::Queued));
				newStateFlags &= ~Job::StateFlags::Queued;
				if (newStateFlags.IsSet(Job::StateFlags::IsExecuting))
				{
					// Job is still executing, skip it
					Job* pSkippedJob = pJob;
					UpdateNextJob();
					pJob = m_pNextJob;
					m_pNextJob = nullptr;
					m_flags.Clear(Flags::HasNextJob);

					const bool isExclusiveToRunner = pSkippedJob->GetAllowedJobRunnerMask() == (1ull << m_threadIndex);
					if (isExclusiveToRunner)
					{
						QueueExclusiveJobFromThread(*pSkippedJob);
					}
					else if (!QueueJobFromThread(*pSkippedJob))
					{
						JobRunnerThread::QueueOnIdealRunner(m_manager, *pSkippedJob);
					}

					if (pJob == nullptr)
					{
						return;
					}

					// Force a new iteration
					previousStateFlags = {};
					continue;
				}

				Assert(!newStateFlags.IsSet(Job::StateFlags::IsExecuting));
				newStateFlags |= Job::StateFlags::IsExecuting;
			} while (!pJob->m_stateFlags.CompareExchangeWeak(previousStateFlags, newStateFlags));
		}

		UpdatePriorityForJob(*pJob);
		const Job::Result result = pJob->OnExecute(*this);

		{
			[[maybe_unused]] const bool cleared = pJob->m_stateFlags.FetchAnd(~Job::StateFlags::IsExecuting).IsSet(Job::StateFlags::IsExecuting);
			Assert(cleared);
		}

		MoveThreadSafeQueueElements();
		UpdateNextJob();

		switch (result)
		{
			case Job::Result::TryRequeue:
			{
				if (m_pNextJob != pJob)
				{
					const bool isExclusiveToRunner = pJob->GetAllowedJobRunnerMask() == (1ull << m_threadIndex);
					if (isExclusiveToRunner)
					{
						pJob->TryQueueExclusiveFromThread(*this);
					}
					else
					{
						pJob->TryQueue(*this);
					}
				}
			}
			break;
			case Job::Result::Finished:
			{
				pJob->SignalExecutionFinished(*this);
			}
			break;
			case Job::Result::AwaitExternalFinish:
			{
				Assert(m_pNextJob != pJob);
				Assert(!pJob->IsQueuedOrExecuting());
				pJob->OnAwaitExternalFinish(*this);
			}
			break;
			case Job::Result::FinishedAndDelete:
			{
				pJob->m_stateFlags |= Job::StateFlags::Destroying;

				Assert(!pJob->IsQueuedOrExecuting());

				pJob->SignalExecutionFinishedAndDestroying(*this);

				Assert(!pJob->IsQueuedOrExecuting());
				Assert(!pJob->HasSubsequentTasks());

				delete pJob;
			}
			break;
			case Job::Result::FinishedAndRunDestructor:
			{
				pJob->m_stateFlags |= Job::StateFlags::Destroying;

				Assert(!pJob->IsQueuedOrExecuting());

				pJob->SignalExecutionFinishedAndDestroying(*this);

				Assert(!pJob->IsQueuedOrExecuting());
				Assert(!pJob->HasSubsequentTasks());

				pJob->~Job();
			}
			break;
		}
	}

	void JobRunnerThread::MoveThreadSafeQueueElements()
	{
		if (m_flags.AreNoneSet(Flags::HasJobsInThreadSafeQueue | Flags::HasJobsInThreadSafeExclusiveQueue))
		{
			return;
		}

		auto moveQueueContents =
			[&threadSafeQueueSize = m_threadSafeQueueSize,
		   &mutex = m_threadSafeQueueMutex,
		   &flags = m_flags](ThreadSafeQueue& threadSafeQueue, auto& targetQueue, const Flags removeFlag, const Flags addFlag) -> bool
		{
			{
				if (threadSafeQueue.IsEmpty())
				{
					return false;
				}

				Threading::UniqueLock lock(mutex);
				if (threadSafeQueue.IsEmpty())
				{
					return false;
				}

				Assert(threadSafeQueueSize >= threadSafeQueue.GetSize());
				threadSafeQueueSize -= threadSafeQueue.GetSize();

				targetQueue.MoveFrom(targetQueue.end(), threadSafeQueue);
				Assert(threadSafeQueue.IsEmpty());
				flags.Clear(removeFlag);
				flags |= addFlag;
			}

			std::sort(
				targetQueue.begin().Get(),
				targetQueue.end().Get(),
				[](const Job& left, const Job& right)
				{
					return left.GetPriority() > right.GetPriority();
				}
			);

			return true;
		};

		const bool movedAnyToShareableQueue =
			moveQueueContents(m_threadSafeQueue, m_queue, Flags::HasJobsInThreadSafeQueue, Flags::HasJobsInLocalQueue);
		const bool movedAnyToExclusiveQueue = moveQueueContents(
			m_threadSafeExclusiveQueue,
			m_exclusiveQueue,
			Flags::HasJobsInThreadSafeExclusiveQueue,
			Flags::HasJobsInLocalExclusiveQueue
		);
		if (movedAnyToShareableQueue)
		{
			GiveWorkToOtherThreads();
		}
		if (movedAnyToShareableQueue | movedAnyToExclusiveQueue)
		{
			UpdateNextJob();
		}
	}

	void JobRunnerThread::UpdateNextJob()
	{
		Assert(IsExecutingOnThread());

		// No next job, queue the next highest priority item we've got
		const EnumFlags<Flags> flags = m_flags.GetFlags();
		if (flags.IsSet(Flags::HasJobsInLocalExclusiveQueue))
		{
			Assert(m_exclusiveQueue.HasElements());
			if (flags.IsSet(Flags::HasJobsInLocalQueue))
			{
				Assert(m_queue.HasElements());
				Threading::Job& exclusiveJob = m_exclusiveQueue.GetLastElement();
				Threading::Job& queuedJob = m_queue.GetLastElement();
				if (exclusiveJob.GetPriority() <= queuedJob.GetPriority())
				{
					if (TrySetNextJob(exclusiveJob))
					{
						m_exclusiveQueue.PopBack();
						m_flags &= ~(Flags::HasJobsInLocalExclusiveQueue * m_exclusiveQueue.IsEmpty());
					}
				}
				else if (TrySetNextJob(queuedJob))
				{
					m_queue.PopBack();
					m_flags &= ~(Flags::HasJobsInLocalQueue * m_queue.IsEmpty());
				}
			}
			else if (TrySetNextJob(m_exclusiveQueue.GetLastElement()))
			{
				m_exclusiveQueue.PopBack();
				m_flags &= ~(Flags::HasJobsInLocalExclusiveQueue * m_exclusiveQueue.IsEmpty());
			}
		}
		else if (flags.IsSet(Flags::HasJobsInLocalQueue))
		{
			Assert(m_queue.HasElements());
			if (TrySetNextJob(m_queue.GetLastElement()))
			{
				m_queue.PopBack();
				m_flags &= ~(Flags::HasJobsInLocalQueue * m_queue.IsEmpty());
			}
		}
	}

	void JobRunnerThread::SetDefaultPriority()
	{
		if (m_flags.IsSet(Flags::CanRunHighPriorityPerformanceJobs))
		{
			if (GetPriority() != Priority::UserInteractive || GetPriorityRatio() != 0.f)
			{
				SetPriority(Priority::UserInteractive, 0.f);
			}
		}
		else if (m_flags.IsSet(Flags::CanRunLowPriorityPerformanceJobs))
		{
			if (GetPriority() != Priority::UserVisibleBackground || GetPriorityRatio() != 0.f)
			{
				SetPriority(Priority::UserVisibleBackground, 0.f);
			}
		}
		else
		{
			Assert(m_flags.IsSet(Flags::CanRunEfficiencyJobs));
			if (GetPriority() != Priority::Background || GetPriorityRatio() != 0.f)
			{
				SetPriority(Priority::Background, 0.f);
			}
		}
	}

	extern Thread::Priority GetThreadPriorityFromJobPriority(const JobPriority jobPriority)
	{
		if (jobPriority >= JobPriority::FirstBackground)
		{
			return Thread::Priority::Background;
		}
		else if (jobPriority >= JobPriority::FirstUserVisibleBackground)
		{
			return Thread::Priority::UserVisibleBackground;
		}
		else if (jobPriority >= JobPriority::FirstUserInitiated)
		{
			return Thread::Priority::UserInitiated;
		}
		else
		{
			return Thread::Priority::UserInteractive;
		}
	}

	void JobRunnerThread::UpdatePriorityForJob(const Job& job)
	{
		const JobPriority jobPriority = job.GetPriority();
		Priority threadPriority;
		float ratio;
		if (jobPriority >= JobPriority::FirstBackground)
		{
			threadPriority = Priority::Background;
			ratio = float(jobPriority - JobPriority::FirstBackground) / float(JobPriority::LastBackground - JobPriority::FirstBackground);
		}
		else if (jobPriority >= JobPriority::FirstUserVisibleBackground)
		{
			threadPriority = Priority::UserVisibleBackground;
			ratio = float(jobPriority - JobPriority::FirstUserVisibleBackground) /
			        float(JobPriority::LastUserVisibleBackground - JobPriority::FirstUserVisibleBackground);
		}
		else if (jobPriority >= JobPriority::FirstUserInitiated)
		{
			threadPriority = Priority::UserInitiated;
			ratio = float(jobPriority - JobPriority::FirstUserInitiated) /
			        float(JobPriority::LastUserInitiated - JobPriority::FirstUserInitiated);
		}
		else
		{
			threadPriority = Priority::UserInteractive;
			ratio = float(jobPriority - JobPriority::FirstUserInteractive) /
			        float(JobPriority::LastUserInteractive - JobPriority::FirstUserInteractive);
		}

		if (ThreadWithRunMember::GetPriority() != threadPriority || ThreadWithRunMember::GetPriorityRatio() != ratio)
		{
			ThreadWithRunMember::SetPriority(threadPriority, ratio);
		}
	}
}

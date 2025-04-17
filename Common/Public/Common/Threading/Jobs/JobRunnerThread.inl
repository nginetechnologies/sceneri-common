#pragma once

#include "JobRunnerThread.h"
#include "JobManager.h"
#include "AsyncJob.h"

namespace ngine::Threading
{
	template<typename Callback>
	inline void JobRunnerThread::QueueCallbackFromThread(const JobPriority priority, Callback&& callback, const ConstStringView name)
	{
		AsyncCallbackJob<Callback>* pJob = new AsyncCallbackJob(priority, Forward<Callback>(callback), name);
		pJob->Queue(*this);
	}

	template<typename Callback>
	inline void JobRunnerThread::QueueExclusiveCallbackFromThread(const JobPriority priority, Callback&& callback, const ConstStringView name)
	{
		AsyncCallbackJob<Callback>* pJob = new AsyncCallbackJob(priority, Forward<Callback>(callback), name);
		pJob->QueueExclusiveFromCurrentThread(*this);
	}

	template<typename Callback>
	inline void
	JobRunnerThread::QueueExclusiveCallbackFromAnyThread(const JobPriority priority, Callback&& callback, const ConstStringView name)
	{
		AsyncCallbackJob<Callback>* pJob = new AsyncCallbackJob(priority, Forward<Callback>(callback), name);
		pJob->QueueExclusiveFromAnyThread(*this);
	}

	template<typename Callback>
	inline void JobManager::QueueCallback(Callback&& callback, const JobPriority priority, const ConstStringView name)
	{
		AsyncCallbackJob<Callback>* pJob = new AsyncCallbackJob(priority, Forward<Callback>(callback), name);
		pJob->Queue(*this);
	}

	inline void JobManager::Queue(const Threading::JobBatch& jobBatch, const Threading::JobPriority jobPriority)
	{
		if (Optional<Threading::Job*> pJob = jobBatch.GetStartJob())
		{
			pJob->Queue(*this);
		}
		else
		{
			Expect(jobBatch.HasStartStage());
			QueueCallback(
				[pStartStage = &jobBatch.GetStartStage()](Threading::JobRunnerThread& thread)
				{
					pStartStage->SignalExecutionFinishedAndDestroying(thread);
				},
				jobPriority
			);
		}
	}
}

#pragma once

#include "Job.h"
#include "JobManager.h"
#include "JobRunnerThread.h"
#include "TimersJob.h"

#include <Common/Time/Duration.h>

namespace ngine::Threading
{
	template<typename Callback>
	struct RecurringAsyncCallbackJob final : public Threading::Job
	{
		RecurringAsyncCallbackJob(const JobPriority priority, const Time::Durationf delay, Callback&& callback)
			: Job(priority)
			, m_delay(delay)
			, m_callback(Forward<Callback>(callback))
		{
		}
		virtual ~RecurringAsyncCallbackJob() = default;

		virtual void OnAwaitExternalFinish(JobRunnerThread& thread) override
		{
			SignalExecutionFinished(thread);
			thread.GetJobManager().ScheduleAsyncJob(m_handle, m_delay, *this);
		}

		[[nodiscard]] Result OnExecute([[maybe_unused]] JobRunnerThread& thread) override
		{
			if (m_callback())
			{
				return Result::AwaitExternalFinish;
			}

			return Result::FinishedAndDelete;
		}

		void SetTimerHandle(TimerHandle&& handle)
		{
			m_handle = Forward<TimerHandle>(handle);
		}
	protected:
		Time::Durationf m_delay;
		Callback m_callback;
		TimerHandle m_handle;
	};

	template<typename Callback>
	inline TimerHandle JobManager::ScheduleRecurringAsync(const Time::Durationf delay, Callback&& callback, const JobPriority priority)
	{
		auto& job = *new RecurringAsyncCallbackJob(priority, delay, Forward<Callback>(callback));
		TimerHandle timerHandle = m_pTimersJob->ReserveHandle(job);
		job.SetTimerHandle(TimerHandle(timerHandle));
		ScheduleAsyncJob(timerHandle, delay, job);
		return timerHandle;
	}
}

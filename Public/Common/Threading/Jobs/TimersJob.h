#pragma once

#include "Job.h"

#include <Common/Time/Duration.h>
#include <Common/Time/Timestamp.h>

#include <Common/Memory/Containers/Vector.h>

#include <Common/Threading/Mutexes/Mutex.h>
#include <Common/Threading/Jobs/TimerHandle.h>

namespace ngine::Threading
{
	//! Job containing a number of timers that will be executed after specific time passes
	//! Guaranteed to not execute the scheduled jobs before the duration, but may execute a bit later than expected
	//! There is only one instance of this job, it runs at the priority of the highest scheduled item
	struct TimersJob final : public Job
	{
		TimersJob();
		virtual ~TimersJob() = default;

		[[nodiscard]] TimerHandle ReserveHandle(Job& job);
		[[nodiscard]] TimerHandle Schedule(const Time::Durationf delay, Job& job, JobManager& jobManager);
		void Schedule(TimerHandle& handle, const Time::Durationf delay, Job& job, JobManager& jobManager);
		bool Cancel(const TimerHandle& handle);
	protected:
		virtual Result OnExecute(JobRunnerThread& thread) override;

		Threading::Mutex m_scheduleLock;
		// TODO: Use multi vector here
		Vector<Time::Durationd> m_scheduleTimes;
		Vector<ReferenceWrapper<Job>> m_scheduleJobs;
	};
}

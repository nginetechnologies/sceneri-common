#include "Threading/Jobs/TimersJob.h"

#include "Threading/Jobs/JobRunnerThread.h"
#include "Threading/Jobs/JobManager.h"

#include <Common/Math/Min.h>
#include <Common/Time/Duration.h>
#include <Common/System/Query.h>

#include <algorithm>

#if PLATFORM_APPLE
#include <dispatch/dispatch.h>
#define USE_NATIVE_TIMER 1
#elif 0 // PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#define USE_NATIVE_TIMER 1
#elif 0 // PLATFORM_POSIX
#include <signal.h>
#include <time.h>
#define USE_NATIVE_TIMER 1
#else
#define USE_NATIVE_TIMER 0
#endif

namespace ngine::Threading
{
	TimersJob::TimersJob()
		: Job(JobPriority::AsyncTimers)
	{
	}

#if USE_NATIVE_TIMER
	struct TimerData
	{
		TimerData() = default;
		TimerData(const TimerData&) = delete;
		TimerData(TimerData&&) = delete;
		TimerData& operator=(const TimerData&) = delete;
		TimerData& operator=(TimerData&&) = delete;
		~TimerData()
		{
#if PLATFORM_WINDOWS
			if (m_timer != nullptr)
			{
				DeleteTimerQueueTimer(m_timerQueue, m_timer, nullptr);
			}

			if (m_timerQueue != nullptr)
			{
				DeleteTimerQueue(m_timerQueue);
			}
#elif PLATFORM_POSIX && !PLATFORM_APPLE
			if (m_timerId != 0)
			{
				timer_delete(m_timerId);
			}
#endif
		}

		enum class Flags : uint8
		{
			Finished = 1 << 0,
			Cancelled = 1 << 1
		};

		AtomicEnumFlags<Flags> m_flags;
		Threading::Atomic<uint32> m_referenceCount{1u};
		Optional<Threading::Job*> m_pJob;

#if PLATFORM_APPLE
		dispatch_block_t m_block{nullptr};
#elif PLATFORM_WINDOWS
		HANDLE m_timerQueue{nullptr};
		HANDLE m_timer{nullptr};
#elif PLATFORM_POSIX
		timer_t m_timerId{0};
#endif
	};
	ENUM_FLAG_OPERATORS(TimerData::Flags);
#endif

	TimerHandle::TimerHandle(TimerHandle&& other)
		: m_pJob(other.m_pJob)
#if USE_NATIVE_TIMER
		, m_pHandle(other.m_pHandle)
#endif
	{
		other.m_pJob = {};
#if USE_NATIVE_TIMER
		other.m_pHandle = nullptr;
#endif
	}

	TimerHandle& TimerHandle::operator=(TimerHandle&& other)
	{
		m_pJob = other.m_pJob;
		other.m_pJob = {};
#if USE_NATIVE_TIMER
		m_pHandle = other.m_pHandle;
		other.m_pHandle = nullptr;
#endif
		return *this;
	}

	TimerHandle::TimerHandle(const TimerHandle& other)
		: m_pJob(other.m_pJob)
#if USE_NATIVE_TIMER
		, m_pHandle(other.m_pHandle)
#endif
	{
#if USE_NATIVE_TIMER
		if (TimerData* pTimerData = reinterpret_cast<TimerData*>(other.m_pHandle))
		{
			pTimerData->m_referenceCount++;
		}
#endif
	}
	TimerHandle& TimerHandle::operator=(const TimerHandle& other)
	{
		m_pJob = other.m_pJob;
#if USE_NATIVE_TIMER
		m_pHandle = other.m_pHandle;
		if (TimerData* pTimerData = reinterpret_cast<TimerData*>(other.m_pHandle))
		{
			pTimerData->m_referenceCount++;
		}
#endif
		return *this;
	}
	TimerHandle::~TimerHandle()
	{
#if USE_NATIVE_TIMER
		if (m_pHandle != nullptr)
		{
			TimerData* pTimerData = reinterpret_cast<TimerData*>(m_pHandle);
			if (pTimerData->m_referenceCount.FetchSubtract(1) == 1)
			{
				delete pTimerData;
			}
		}
#endif
	}

	TimerHandle TimersJob::ReserveHandle(Job& job)
	{
#if USE_NATIVE_TIMER
		TimerData* pTimerData = new TimerData();
		return TimerHandle{job, reinterpret_cast<void*>(pTimerData)};
#else
		return TimerHandle{job, nullptr};
#endif
	}

	TimerHandle TimersJob::Schedule(const Time::Durationf delay, Job& job, JobManager& jobManager)
	{
		TimerHandle handle = ReserveHandle(job);
		Schedule(handle, delay, job, jobManager);
		return handle;
	}

	void TimersJob::Schedule([[maybe_unused]] TimerHandle& handle, const Time::Durationf delay, Job& job, JobManager& jobManager)
	{
		if (!handle.IsValid())
		{
			handle = ReserveHandle(job);
		}

#if USE_NATIVE_TIMER
		TimerData* pTimerData = reinterpret_cast<TimerData*>(handle.m_pHandle);
		pTimerData->m_flags &= ~(TimerData::Flags::Finished | TimerData::Flags::Cancelled);
		pTimerData->m_referenceCount++;
		pTimerData->m_pJob = job;

		jobManager.StartExternalTask();
#if PLATFORM_APPLE
		qos_class_t qualityOfServiceClass;
		switch (GetThreadPriorityFromJobPriority(job.GetPriority()))
		{
			case Thread::Priority::UserInteractive:
				qualityOfServiceClass = QOS_CLASS_USER_INTERACTIVE;
				break;
			case Thread::Priority::UserInitiated:
				qualityOfServiceClass = QOS_CLASS_USER_INITIATED;
				break;
			case Thread::Priority::Default:
				qualityOfServiceClass = QOS_CLASS_DEFAULT;
				break;
			case Thread::Priority::UserVisibleBackground:
				qualityOfServiceClass = QOS_CLASS_UTILITY;
				break;
			case Thread::Priority::Background:
				qualityOfServiceClass = QOS_CLASS_BACKGROUND;
				break;
		}
		const dispatch_block_t block =
			dispatch_block_create_with_qos_class(DISPATCH_BLOCK_ENFORCE_QOS_CLASS, qualityOfServiceClass, 0, ^(void) {
				const EnumFlags<TimerData::Flags> previousFlags = pTimerData->m_flags.FetchOr(TimerData::Flags::Finished);
				Assert(previousFlags.IsNotSet(TimerData::Flags::Finished));
				if (previousFlags.IsNotSet(TimerData::Flags::Cancelled))
				{
					pTimerData->m_pJob->TryQueue(jobManager);
					jobManager.StopExternalTask();
				}

				if (pTimerData->m_referenceCount.FetchSubtract(1) == 1)
				{
					delete pTimerData;
				}
			});
		pTimerData->m_block = block;

		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delay.GetNanoseconds()), dispatch_get_global_queue(qualityOfServiceClass, 0), block);
#elif PLATFORM_WINDOWS
		pTimerData->m_timerQueue = CreateTimerQueue();
		const auto timerCallback = [](PVOID lpParam, [[maybe_unused]] BOOLEAN TimerOrWaitFired)
		{
			TimerData* pTimerData = reinterpret_cast<TimerData*>(lpParam);

			const EnumFlags<TimerData::Flags> previousFlags = pTimerData->m_flags.FetchOr(TimerData::Flags::Finished);
			Assert(previousFlags.IsNotSet(TimerData::Flags::Finished));
			if (previousFlags.IsNotSet(TimerData::Flags::Cancelled))
			{
				JobManager& jobManager = System::Get<JobManager>();
				pTimerData->m_pJob->TryQueue(jobManager);
				jobManager.StopExternalTask();
			}

			if (pTimerData->m_referenceCount.FetchSubtract(1) == 1)
			{
				delete pTimerData;
			}
		};

		[[maybe_unused]] const bool createdTimer = CreateTimerQueueTimer(
			&pTimerData->m_timer,
			pTimerData->m_timerQueue,
			timerCallback,
			pTimerData,
			(DWORD)delay.GetMilliseconds(),
			0,
			WT_EXECUTEDEFAULT
		);
		Assert(createdTimer);
#elif PLATFORM_POSIX
		struct sigaction sigAction;
		sigAction.sa_flags = SA_SIGINFO;
		sigAction.sa_sigaction = []([[maybe_unused]] const int signal, siginfo_t* pSigInfo, [[maybe_unused]] void* uc)
		{
			TimerData* pTimerData = reinterpret_cast<TimerData*>(pSigInfo->si_value.sival_ptr);

			const EnumFlags<TimerData::Flags> previousFlags = pTimerData->m_flags.FetchOr(TimerData::Flags::Finished);
			Assert(previousFlags.IsNotSet(TimerData::Flags::Finished));
			if (previousFlags.IsNotSet(TimerData::Flags::Cancelled))
			{
				JobManager& jobManager = System::Get<JobManager>();
				pTimerData->m_pJob->TryQueue(jobManager);
				jobManager.StopExternalTask();
			}

			if (pTimerData->m_referenceCount.FetchSubtract(1) == 1)
			{
				delete pTimerData;
			}
		};
		sigemptyset(&sigAction.sa_mask);
		[[maybe_unused]] const auto actionResult = sigaction(SIGRTMIN, &sigAction, nullptr);
		Assert(actionResult == 0);

		struct sigevent sigEvent;

		sigEvent.sigev_notify = SIGEV_SIGNAL;
		sigEvent.sigev_signo = SIGRTMIN;
		sigEvent.sigev_value.sival_ptr = pTimerData;

		[[maybe_unused]] const auto result = timer_create(CLOCK_REALTIME, &sigEvent, &pTimerData->m_timerId);
		Assert(result == 0);

		struct itimerspec timerSpec;
		timerSpec.it_value.tv_sec = (long int)delay.GetSeconds();
		timerSpec.it_value.tv_nsec = (long int)(delay.GetNanoseconds() % 1'000'000'000);
		timerSpec.it_interval.tv_sec = 0;
		timerSpec.it_interval.tv_nsec = 0;

		[[maybe_unused]] const auto timerResult = timer_settime(pTimerData->m_timerId, 0, &timerSpec, nullptr);
		Assert(timerResult == 0);
#endif

#else
		const Time::Durationd scheduledTime = Time::Durationd::GetCurrentSystemUptime() + delay;

		{
			Threading::UniqueLock lock(m_scheduleLock);
			Time::Durationd* upperBound = std::upper_bound(
				m_scheduleTimes.begin().Get(),
				m_scheduleTimes.end().Get(),
				scheduledTime,
				[](const Time::Durationd newScheduledTime, const Time::Durationd existingScheduledTime) -> bool
				{
					return newScheduledTime < existingScheduledTime;
				}
			);

			const uint32 index = m_scheduleTimes.GetIteratorIndex(upperBound);
			m_scheduleTimes.Emplace(upperBound, Memory::Uninitialized, scheduledTime);
			m_scheduleJobs.Emplace(m_scheduleJobs.begin() + index, Memory::Uninitialized, job);
		}

		/*if (!IsQueued())
		{
		   JobPriority newPriority = JobPriority::Lowest;
		   for (const Threading::Job& scheduledJob : m_scheduleJobs)
		   {
		   newPriority = Math::Min(newPriority, scheduledJob.GetPriority());
		   }
		   SetPriority(newPriority);
		}*/
		TryQueue(jobManager);
#endif
	}

	bool TimersJob::Cancel(const TimerHandle& __restrict handle)
	{
		Assert(handle.IsValid());

#if USE_NATIVE_TIMER
		TimerData* pTimerData = reinterpret_cast<TimerData*>(handle.m_pHandle);
		const EnumFlags<TimerData::Flags> previousFlags = pTimerData->m_flags.FetchOr(TimerData::Flags::Cancelled);
		if (previousFlags.IsSet(TimerData::Flags::Cancelled))
		{
			return false;
		}

		if (previousFlags.IsNotSet(TimerData::Flags::Finished))
		{
#if PLATFORM_APPLE
			Assert(dispatch_block_testcancel(pTimerData->m_block) == 0, "Block was already cancelled!");
			dispatch_block_cancel(pTimerData->m_block);
			return dispatch_block_testcancel(pTimerData->m_block) != 0;
#elif PLATFORM_WINDOWS
			return DeleteTimerQueueTimer(&pTimerData->m_timer, pTimerData->m_timerQueue, nullptr);
#elif PLATFORM_POSIX
			return timer_delete(pTimerData->m_timerId) == 0;
#endif
		}
		else
		{
			return false;
		}
#else
		Threading::UniqueLock lock(m_scheduleLock);
		auto it = m_scheduleJobs.Find(*handle.m_pJob);
		if (it != m_scheduleJobs.end())
		{
			Assert(!handle.m_pJob->IsQueuedOrExecuting());
			const uint32 index = m_scheduleJobs.GetIteratorIndex(it);
			auto timeIt = m_scheduleTimes.begin() + index;
			if (Ensure(timeIt < m_scheduleTimes.end()))
			{
				m_scheduleTimes.Remove(timeIt);
			}
			m_scheduleJobs.Remove(it);
			Assert(!handle.m_pJob->IsQueuedOrExecuting());
			return true;
		}
		else
		{
			return false;
		}
#endif
	}

	Job::Result TimersJob::OnExecute(JobRunnerThread& thread)
	{
		const Time::Durationd currentTime = Time::Durationd::GetCurrentSystemUptime();

		Threading::UniqueLock lock(m_scheduleLock);
		decltype(m_scheduleTimes)::const_iterator scheduledJobIterator = std::lower_bound(
			m_scheduleTimes.begin().Get(),
			m_scheduleTimes.end().Get(),
			currentTime,
			[](const Time::Durationd scheduledTime, const Time::Durationd currentTime) -> bool
			{
				return scheduledTime < currentTime;
			}
		);

		const decltype(m_scheduleJobs)::View scheduleView = {m_scheduleJobs.begin(), m_scheduleTimes.GetIteratorIndex(scheduledJobIterator)};

		if (scheduleView.HasElements())
		{
			thread.TryQueueJobsFromThread(scheduleView);

			m_scheduleJobs.Remove(scheduleView);

			const decltype(m_scheduleTimes)::View scheduleTimestampsView = {m_scheduleTimes.begin(), scheduledJobIterator};

			m_scheduleTimes.Remove(scheduleTimestampsView);

			/*if (!IsQueued())
			{
			  JobPriority newPriority = JobPriority::Lowest;
			  for (const Threading::Job& scheduledJob : m_scheduleJobs)
			  {
			  newPriority = Math::Min(newPriority, scheduledJob.GetPriority());
			  }
			  SetPriority(newPriority);
			}*/

			return m_scheduleJobs.HasElements() ? Result::TryRequeue : Result::Finished;
		}

		// Assert(m_scheduleJobs.HasElements());
		return Result::TryRequeue;
	}
}

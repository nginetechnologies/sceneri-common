#pragma once

#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Containers/FlatVector.h>
#include <Common/Memory/UniqueRef.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/CountBits.h>
#include <Common/Threading/ThreadId.h>
#include <Common/Threading/AtomicInteger.h>
#include <Common/Threading/Jobs/JobBatch.h>
#include <Common/Threading/Jobs/JobRunnerMask.h>
#include <Common/Threading/Jobs/TimerHandle.h>
#include <Common/Time/ForwardDeclarations/Duration.h>
#include <Common/Math/Range.h>
#include <Common/System/SystemType.h>

#include "JobPriority.h"

namespace ngine::Threading
{
	struct Job;
	struct TimersJob;
	struct JobRunnerThread;

	//! Work sharing based job system
	struct JobManager
	{
		inline static constexpr System::Type SystemType = System::Type::JobManager;

		JobManager();
		JobManager(const JobManager&) = delete;
		JobManager& operator=(const JobManager&) = delete;
		JobManager(JobManager&& other) = delete;
		JobManager& operator=(JobManager&&) = delete;
		virtual ~JobManager();

		void StartRunners();
		void StartRunners(
			const uint16 count,
			const Math::Range<uint16> performanceHighPriorityWorkerRange,
			const Math::Range<uint16> performanceLowPriorityWorkerRange,
			const Math::Range<uint16> efficiencyWorkerRange
		);
		void StartRunners(const uint16 performanceCoreCount, const uint16 efficiencyCoreCount)
		{
			StartRunners(
				performanceCoreCount + efficiencyCoreCount,
				Math::Range<uint16>::MakeStartToEnd(0, performanceCoreCount),
				Math::Range<uint16>::MakeStartToEnd(0, performanceCoreCount),
				Math::Range<uint16>::MakeStartToEnd(performanceCoreCount, uint16(performanceCoreCount + efficiencyCoreCount))
			);
		}

		using ConstJobThreadView = ArrayView<ReferenceWrapper<const JobRunnerThread>, uint8>;
		using JobThreadView = ArrayView<ReferenceWrapper<JobRunnerThread>, uint8>;

		[[nodiscard]] ConstJobThreadView GetJobThreads() const
		{
			ArrayView<const ReferenceWrapper<JobRunnerThread>, uint8> jobThreads = m_jobThreads;
			return *reinterpret_cast<ConstJobThreadView*>(&jobThreads);
		}
		[[nodiscard]] JobThreadView GetJobThreads()
		{
			return m_jobThreads;
		}

		[[nodiscard]] Optional<JobRunnerThread*> GetJobThread(const ThreadId id) const
		{
			return m_jobThreadIdLookup[GetJobThreadIndexFromId(id)];
		}

		bool MarkThreadAsIdle(const uint8 threadIndex)
		{
			const JobRunnerMask threadMask = 1ull << threadIndex;
			const JobRunnerMask previousMask = m_idleThreadMask.FetchOr(threadMask);
			return (previousMask & threadMask) == 0;
		}
		bool ClearThreadIdleFlag(const uint8 threadIndex)
		{
			const JobRunnerMask threadMask = 1ull << threadIndex;
			const JobRunnerMask previousMask = m_idleThreadMask.FetchAnd(~threadMask);
			return (previousMask & threadMask) != 0;
		}
		[[nodiscard]] bool IsThreadIdle(const uint8 threadIndex) const
		{
			return m_idleThreadMask & (1ull << threadIndex);
		}

		[[nodiscard]] JobRunnerMask GetPerformanceHighPriorityThreadMask() const
		{
			return m_performanceHighPriorityThreadMask;
		}
		[[nodiscard]] JobRunnerMask GetPerformanceLowPriorityThreadMask() const
		{
			return m_performanceLowPriorityThreadMask;
		}
		[[nodiscard]] JobRunnerMask GetEfficiencyThreadMask() const
		{
			return m_efficiencyThreadMask;
		}

		[[nodiscard]] PURE_STATICS JobRunnerMask GetPreferredRunnerMask(JobPriority jobPriority)
		{
			// Job did not filter for any specific runners, select based on priority
			const bool isHighPriorityPerformanceJob = jobPriority < JobPriority::FirstUserVisibleBackground;
			const bool isLowPriorityPerformanceJob = jobPriority >= JobPriority::FirstUserVisibleBackground;

			return isHighPriorityPerformanceJob ? m_performanceHighPriorityThreadMask
			                                    : (isLowPriorityPerformanceJob ? m_performanceLowPriorityThreadMask : m_efficiencyThreadMask);
		}

		[[nodiscard]] JobRunnerMask StealIdleThreads(uint8 maxCount, const JobRunnerMask allowedMask)
		{
			if (maxCount == 0)
			{
				return 0;
			}

			JobRunnerMask stolenIdleThreadMask = 0u;
			JobRunnerMask iteratedMask = m_idleThreadMask & allowedMask;
			for (const uint8 threadIndex : Memory::GetSetBitsIterator(iteratedMask))
			{
				const JobRunnerMask threadMask = 1ull << threadIndex;

				JobRunnerMask storedMask = iteratedMask | (m_idleThreadMask & ~allowedMask);

				if (m_idleThreadMask.CompareExchangeStrong(storedMask, storedMask & ~threadMask))
				{
					stolenIdleThreadMask |= threadMask;
					iteratedMask &= ~threadMask;
					maxCount--;
					if (maxCount == 0)
					{
						return stolenIdleThreadMask;
					}
				}
				else
				{
					// Mask changed, settle for what we got so far
					return stolenIdleThreadMask;
				}
			}

			return stolenIdleThreadMask;
		}
		[[nodiscard]] Optional<uint8> StealFirstIdleThread(const JobRunnerMask allowedMask)
		{
			const JobRunnerMask mask = StealIdleThreads(1, allowedMask);
			const uint8 index = (uint8)Memory::GetNumberOfTrailingZeros(mask);
			return Optional<uint8>(index, mask != 0);
		}
		[[nodiscard]] void ReturnStolenIdleThreads(const JobRunnerMask mask)
		{
			m_idleThreadMask |= mask;
		}

		[[nodiscard]] bool AreAllRunnersIdle() const
		{
			return Memory::GetNumberOfSetBits(m_idleThreadMask) == m_jobThreads.GetSize();
		}
		[[nodiscard]] uint8 GetNumberOfIdleThreads() const
		{
			return (uint8)Memory::GetNumberOfSetBits(m_idleThreadMask);
		}

		[[nodiscard]] TimerHandle ScheduleAsyncJob(const Time::Durationf delay, Job& job);
		void ScheduleAsyncJob(TimerHandle& handle, const Time::Durationf delay, Job& job);

		template<typename Callback>
		inline TimerHandle ScheduleAsync(const Time::Durationf delay, Callback&& callback, const JobPriority priority);
		template<typename Callback>
		inline TimerHandle ScheduleRecurringAsync(const Time::Durationf delay, Callback&& callback, const JobPriority priority);

		bool CancelAsyncJob(const TimerHandle& handle);

		void StartExternalTask()
		{
			m_externalTaskCounter++;
		}
		void StopExternalTask()
		{
			m_externalTaskCounter--;
		}
		[[nodiscard]] bool IsRunningExternalTasks() const
		{
			return m_externalTaskCounter > 0;
		}

		template<typename Callback>
		void QueueCallback(Callback&& callback, const JobPriority priority, const ConstStringView name = {});

		void Queue(const Threading::JobBatch& jobBatch, const Threading::JobPriority jobPriority);
	protected:
		[[nodiscard]] PURE_LOCALS_AND_POINTERS uint8 GetJobThreadIndexFromId(const ThreadId id) const;

		virtual void CreateJobRunners(const uint16 count);
		virtual void DestroyJobRunners();
	protected:
		void* m_pJobThreadData = nullptr;
		// Currently limited by Windows API
		inline static constexpr uint8 MaximumRunnerCount = 64;
		using JobThreads = FlatVector<ReferenceWrapper<JobRunnerThread>, MaximumRunnerCount>;
		JobThreads m_jobThreads;
		Array<JobRunnerThread*, MaximumRunnerCount> m_jobThreadIdLookup = {};
		Threading::Atomic<JobRunnerMask> m_idleThreadMask = 0;
		// Mask indicating which threads low priority and potentially slow tasks can run on
		// This is also used to indicate the opposite, very high priority tasks will not be allowed on low priority threads
		JobRunnerMask m_performanceLowPriorityThreadMask = 0;
		JobRunnerMask m_performanceHighPriorityThreadMask = 0;
		JobRunnerMask m_efficiencyThreadMask = 0;
		//! Number of tasks that are currently executing outside of the job system
		//! For example used by OS-level timer scheduling
		Threading::Atomic<uint64> m_externalTaskCounter = 0;

		UniqueRef<TimersJob> m_pTimersJob;
	};
}

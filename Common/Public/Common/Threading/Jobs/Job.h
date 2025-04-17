#pragma once

#include "StageBase.h"
#include "JobPriority.h"
#include "JobRunnerMask.h"
#include "CallbackResult.h"

#include <Common/EnumFlagOperators.h>

#include <Common/AtomicEnumFlags.h>
#include <Common/Threading/AtomicEnum.h>
#include <Common/Threading/AtomicInteger.h>

namespace ngine::Threading
{
	struct JobManager;

	enum class JobStateFlags : uint8
	{
		None,
		Queued = 1 << 0,
		IsExecuting = 1 << 1,
		Destroying = 1 << 2
	};
	ENUM_FLAG_OPERATORS(JobStateFlags);

	//! Represents a step of the work that needs to be done for a scene in a given frame
	//! An example of a stage can be component type update, a render stage, or
	//! Stages can depend on each other, so that a dependee only runs when it's dependencies are ready
	//! Multiple stages can run concurrently, but not before their own dependencies
	//! Typical first scene stage will be the camera component for each view, in order to resolve the first dependency
	struct Job : public StageBase
	{
		using Priority = JobPriority;
		using Result = CallbackResult;
		using StateFlags = JobStateFlags;

		Job(const Priority priority, const JobRunnerMask allowedJobRunnerMask = Math::NumericLimits<JobRunnerMask>::Max)
			: m_priority(priority)
			, m_allowedJobRunnerMask(allowedJobRunnerMask)
		{
		}
		Job(const Job&) = delete;
		Job& operator=(const Job&) = delete;
		Job(Job&& other) = delete;
		Job& operator=(Job&&) = delete;
		virtual ~Job()
		{
			Assert(!IsQueuedOrExecuting());
		}

		//! Queues the job from the current thread
		//! Must not be called if the job was already queued
		void Queue(JobRunnerThread& thread);
		//! Queues the job from an unknown thread
		//! Must not be called if the job was already queued
		void Queue(JobManager& manager);
		//! Queues the job on a specific thread
		//! The job is only allowed to execute on the specified thread
		//! Must not be called if the job was already queued
		void QueueExclusiveFromCurrentThread(JobRunnerThread& thread);
		//! Queues the job on a specific thread
		//! The job is only allowed to execute on the specified thread
		//! Must not be called if the job was already queued
		void QueueExclusiveFromAnyThread(JobRunnerThread& thread);
		//! Queues the job if it was not already queued
		void TryQueue(JobRunnerThread& thread);
		//! Queues the job if it was not already queued
		void TryQueue(JobManager& manager);
		//! Queues the job if it was not already queued
		//! The job is only allowed to execute on the specified thread
		void TryQueueExclusiveFromThread(JobRunnerThread& thread);
		//! Queues the job if it was not already queued
		//! The job is only allowed to execute on the specified thread
		void TryQueueExclusiveFromAnyThread(JobRunnerThread& thread);
		[[nodiscard]] PURE_STATICS bool IsQueued() const
		{
			return m_stateFlags.IsSet(StateFlags::Queued);
		}
		[[nodiscard]] PURE_STATICS bool IsExecuting() const
		{
			return m_stateFlags.IsSet(StateFlags::IsExecuting);
		}
		[[nodiscard]] PURE_STATICS bool IsQueuedOrExecuting() const
		{
			return m_stateFlags.AreAnySet(StateFlags::Queued | StateFlags::IsExecuting);
		}

		[[nodiscard]] PURE_STATICS Priority GetPriority() const
		{
			return m_priority;
		}
		[[nodiscard]] PURE_STATICS JobRunnerMask GetAllowedJobRunnerMask() const
		{
			return m_allowedJobRunnerMask;
		}

		[[nodiscard]] PURE_STATICS bool IsLowPriorityPerformanceJob() const
		{
			return m_priority >= Priority::FirstUserVisibleBackground;
		}
		[[nodiscard]] PURE_STATICS bool IsHighPriorityPerformanceJob() const
		{
			return m_priority < Priority::FirstUserVisibleBackground;
		}
		[[nodiscard]] PURE_STATICS bool IsEfficiencyJob() const
		{
			return m_priority >= Priority::FirstBackground;
		}
	protected:
		friend JobRunnerThread;

		void FlagQueued();

		virtual void OnDependenciesResolved(Threading::JobRunnerThread& thread) override
		{
			Queue(thread);
		}

		[[nodiscard]] virtual Result OnExecute(JobRunnerThread& thread) = 0;

		void SetPriority(const Priority priority)
		{
			Assert(!IsQueued(), "Job priority must not be changed while queued!");
			m_priority = priority;
		}
		void SetAllowedJobRunnerMask(const JobRunnerMask mask)
		{
			m_allowedJobRunnerMask = mask;
		}
		void SetExclusiveToThread(JobRunnerThread& thread);
	private:
		Atomic<Priority> m_priority;
		AtomicEnumFlags<StateFlags> m_stateFlags;
		Atomic<JobRunnerMask> m_allowedJobRunnerMask{Math::NumericLimits<JobRunnerMask>::Max};
	};
}

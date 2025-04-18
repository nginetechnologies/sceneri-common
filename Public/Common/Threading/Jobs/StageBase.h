#pragma once

#include "JobPriority.h"

#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/ReferenceWrapper.h>

#include <Common/Threading/AtomicInteger.h>
#include <Common/EnumFlags.h>
#include <Common/TypeTraits/HasFunctionCallOperator.h>

#define STAGE_DEPENDENCY_DEBUG 0
#define STAGE_DEPENDENCY_PROFILING PROFILE_BUILD

#if STAGE_DEPENDENCY_PROFILING
#include <Common/Memory/Containers/ZeroTerminatedStringView.h>
#endif

#include <Common/Threading/Mutexes/SharedMutex.h>

namespace ngine::Threading
{
	struct JobRunnerThread;

	struct StageBase
	{
		virtual ~StageBase()
		{
#if STAGE_DEPENDENCY_DEBUG
			{
				Threading::UniqueLock lock(m_dependencyLock);
				for (Threading::StageBase& dependency : m_dependencies)
				{
					[[maybe_unused]] const bool wasNextStageRemoved = dependency.m_nextJobs.RemoveFirstOccurrence(*this);
					Assert(wasNextStageRemoved);
				}
			}
			{
				Threading::UniqueLock lock(m_nextJobMutex);
				for (Threading::StageBase& nextStage : m_nextJobs)
				{
					[[maybe_unused]] const bool wasNextStageRemoved = nextStage.m_dependencies.RemoveFirstOccurrence(*this);
					Assert(wasNextStageRemoved);
				}
			}
#endif
		}
		using DependencySizeType = uint16;

#if STAGE_DEPENDENCY_PROFILING
		[[nodiscard]] virtual ConstZeroTerminatedStringView GetDebugName() const
		{
			return {};
		}
#endif

		inline void AddSubsequentStage(StageBase& stage)
		{
			if (Ensure(!IsDirectlyFollowedBy(stage)))
			{
#if STAGE_DEPENDENCY_DEBUG
				{
					Threading::UniqueLock lock(stage.m_dependencyLock);
					Threading::UniqueLock nextJobsLock(m_nextJobMutex);
					stage.m_dependencies.EmplaceBack(*this);
					;
					stage.m_remainingDependencies.EmplaceBack(*this);
					m_nextJobs.EmplaceBack(stage);
				}

				OnNextStageChanged();
				stage.OnDependencyChanged();
#else
				stage.m_dependencyCount++;
				Threading::UniqueLock lock(m_nextJobMutex);
				m_nextJobs.EmplaceBack(stage);
#endif
			}
		}

		template<typename Callback, typename = EnableIf<TypeTraits::HasFunctionCallOperator<Callback>>>
		void AddSubsequentStage(Callback&& callback, const JobPriority priority, [[maybe_unused]] const ConstStringView name = {});

#if STAGE_DEPENDENCY_DEBUG
		virtual void OnDependencyChanged() const
		{
			for (const Threading::StageBase& dependency : GetDependencies())
			{
				dependency.OnDependencyChanged();
			}
		}

		virtual void OnNextStageChanged() const
		{
			for (const Threading::StageBase& dependency : GetNextStages())
			{
				dependency.OnNextStageChanged();
			}
		}
#endif

		enum class RemovalFlags : uint8
		{
			WasCompleted = 1 << 0,
			ExecuteIfDependenciesResolved = 1 << 1,
			Default = ExecuteIfDependenciesResolved
		};

		void RemoveSubsequentStage(
			StageBase& stage, const Optional<JobRunnerThread*> pThread, const EnumFlags<RemovalFlags> flags = RemovalFlags::Default
		)
		{
			if (Ensure(IsDirectlyFollowedBy(stage)))
			{
#if STAGE_DEPENDENCY_DEBUG
				{
					Threading::UniqueLock lock(stage.m_dependencyLock);
					Threading::UniqueLock nextJobLock(m_nextJobMutex);
					[[maybe_unused]] const bool wasCompleted = stage.m_completedDependencies.GetSize() == stage.m_dependencies.GetSize();
					[[maybe_unused]] const bool wasRemoved = stage.m_dependencies.RemoveFirstOccurrence(*this);
					Assert(wasRemoved);
					{
						[[maybe_unused]] const bool wasNextStageRemoved = m_nextJobs.RemoveFirstOccurrence(stage);
						Assert(wasNextStageRemoved);
					}

					decltype(m_completedDependencies)::OptionalIteratorType pCompletedIt = stage.m_completedDependencies.Find(*this);
					Assert(pCompletedIt.IsValid() == flags.IsSet(RemovalFlags::WasCompleted));
					if (pCompletedIt)
					{
						Assert(!stage.m_remainingDependencies.Contains(*this));
						stage.m_completedDependencies.Remove(pCompletedIt);
						Assert(stage.m_completedDependencies.GetSize() <= stage.m_dependencies.GetSize());
						if (stage.m_remainingDependencies.IsEmpty() && stage.m_dependencies.IsEmpty() && flags.IsSet(RemovalFlags::ExecuteIfDependenciesResolved))
						{
							Assert(!wasCompleted);
							lock.Unlock();
							stage.OnDependenciesResolvedInternal(*pThread);
						}
					}
					else
					{
						[[maybe_unused]] const bool wasRemainingDependencyRemoved = stage.m_remainingDependencies.RemoveFirstOccurrence(*this);
						Assert(wasRemainingDependencyRemoved);
						if (stage.m_remainingDependencies.IsEmpty() && flags.IsSet(RemovalFlags::ExecuteIfDependenciesResolved))
						{
							Assert(!wasCompleted);
							lock.Unlock();
							stage.OnDependenciesResolvedInternal(*pThread);
						}
					}
				}

				OnDependencyChanged();
#else
				[[maybe_unused]] const bool wasCompleted = stage.AreAllDependenciesResolved();
				const DependencySizeType previousDependencyCount = stage.m_dependencyCount.FetchSubtract(1);
				Assert(!flags.IsSet(RemovalFlags::WasCompleted) || stage.m_completedDependencyCount.Load() > 0);
				const DependencySizeType previousCompletedDependencyCount =
					stage.m_completedDependencyCount.FetchSubtract(flags.IsSet(RemovalFlags::WasCompleted));
				{
					Threading::UniqueLock lock(m_nextJobMutex);
					[[maybe_unused]] const bool wasNextStageRemoved = m_nextJobs.RemoveFirstOccurrence(stage);
					Assert(wasNextStageRemoved);
				}
				if (flags.IsSet(RemovalFlags::WasCompleted))
				{
					if (previousDependencyCount == 1 && flags.IsSet(RemovalFlags::ExecuteIfDependenciesResolved))
					{
						Assert(!wasCompleted);
						stage.OnDependenciesResolvedInternal(*pThread);
					}
				}
				else
				{
					if (previousCompletedDependencyCount == previousDependencyCount - 1)
					{
						if (flags.IsSet(RemovalFlags::ExecuteIfDependenciesResolved))
						{
							Assert(!wasCompleted);
							stage.OnDependenciesResolved(*pThread);
						}
					}
				}
#endif
			}
		}

		[[nodiscard]] inline bool IsDirectlyFollowedBy(const StageBase& stage) const
		{
			Threading::SharedLock lock(m_nextJobMutex);
			return m_nextJobs.Contains(stage);
		}

		[[nodiscard]] inline bool IsIndirectlyFollowedBy(const StageBase& stage) const
		{
			Threading::SharedLock lock(m_nextJobMutex);
			if (m_nextJobs.Contains(stage))
			{
				return true;
			}

			for (const StageBase& nextJob : m_nextJobs)
			{
				if (nextJob.IsIndirectlyFollowedBy(stage))
				{
					return true;
				}
			}

			return false;
		}

		[[nodiscard]] bool HasDependencies() const
		{
#if STAGE_DEPENDENCY_DEBUG
			Threading::SharedLock lock(m_dependencyLock);
			return m_dependencies.HasElements();
#else
			return m_dependencyCount > 0;
#endif
		}

		[[nodiscard]] bool HasSubsequentTasks() const
		{
			Threading::SharedLock lock(m_nextJobMutex);
			return m_nextJobs.HasElements();
		}

		[[nodiscard]] DependencySizeType GetDependencyCount() const
		{
#if STAGE_DEPENDENCY_DEBUG
			Threading::SharedLock lock(m_dependencyLock);
			return m_dependencies.GetSize();
#else
			return m_dependencyCount;
#endif
		}
		[[nodiscard]] DependencySizeType GetCompletedDependencyCount() const
		{
#if STAGE_DEPENDENCY_DEBUG
			Threading::SharedLock lock(m_dependencyLock);
			return m_completedDependencies.GetSize();
#else
			return m_completedDependencyCount;
#endif
		}

#if STAGE_DEPENDENCY_DEBUG
		struct DependencyView : public ArrayView<const ReferenceWrapper<StageBase>>
		{
			using BaseType = ArrayView<const ReferenceWrapper<StageBase>>;
			DependencyView(
				Threading::SharedLock<Threading::SharedMutex>&& lock, const typename BaseType::iterator begin, const typename BaseType::iterator end
			)
				: BaseType(begin, end)
				, m_lock(Forward<Threading::SharedLock<Threading::SharedMutex>>(lock))
			{
				if (UNLIKELY_ERROR(!m_lock.IsLocked()))
				{
					BaseType::operator=({});
				}
			}
			DependencyView(DependencyView&& other) = default;
			DependencyView(const DependencyView&) = delete;
			DependencyView& operator=(DependencyView&&) = default;
			DependencyView& operator=(const DependencyView&) = delete;
		private:
			Threading::SharedLock<Threading::SharedMutex> m_lock;
		};

		[[nodiscard]] DependencyView GetRemainingDependencies() const LIFETIME_BOUND
		{
			Threading::SharedLock lock(m_dependencyLock);
			return {Move(lock), m_remainingDependencies.begin(), m_remainingDependencies.end()};
		}
		[[nodiscard]] DependencyView GetDependencies() const LIFETIME_BOUND
		{
			Threading::SharedLock lock(m_dependencyLock);
			return {Move(lock), m_dependencies.begin(), m_dependencies.end()};
		}
		[[nodiscard]] DependencyView GetCompletedDependencies() const LIFETIME_BOUND
		{
			Threading::SharedLock lock(m_dependencyLock);
			return {Move(lock), m_completedDependencies.begin(), m_completedDependencies.end()};
		}
		[[nodiscard]] DependencyView GetNextStages() const LIFETIME_BOUND
		{
			Threading::SharedLock lock(m_nextJobMutex);
			return {Move(lock), m_nextJobs.begin(), m_nextJobs.end()};
		}
#endif

		[[nodiscard]] bool AreAllDependenciesResolved() const
		{
#if STAGE_DEPENDENCY_DEBUG
			Threading::SharedLock lock(m_dependencyLock);
			return m_completedDependencies.GetSize() == m_dependencies.GetSize();
#else
			return m_completedDependencyCount == m_dependencyCount;
#endif
		}

		inline void OnDependencyExecuted(JobRunnerThread& thread, [[maybe_unused]] StageBase& dependency)
		{
#if STAGE_DEPENDENCY_DEBUG
			DependencySizeType previousCompletedDependencyCount;
			Threading::UniqueLock lock(m_dependencyLock);
			{
				previousCompletedDependencyCount = m_completedDependencies.GetSize();
				Assert(m_dependencies.Contains(dependency));
				Assert(!m_completedDependencies.Contains(dependency));
				m_completedDependencies.EmplaceBack(dependency);
				[[maybe_unused]] const bool removed = m_remainingDependencies.RemoveFirstOccurrence(dependency);
				Assert(removed);
			}
			if (previousCompletedDependencyCount + 1u == m_dependencies.GetSize())
			{
				lock.Unlock();
				OnDependenciesResolvedInternal(thread);
			}
#else
			Assert(m_completedDependencyCount < Math::NumericLimits<DependencySizeType>::Max);
			const DependencySizeType previousCompletedDependencyCount = m_completedDependencyCount.FetchAdd(1u);
			if (previousCompletedDependencyCount + 1u == m_dependencyCount)
			{
				OnDependenciesResolvedInternal(thread);
			}
#endif
		}

		void SignalExecutionFinished(JobRunnerThread& thread)
		{
			{
				Threading::SharedLock lock(m_nextJobMutex);
				for (StageBase& __restrict nextJob : m_nextJobs)
				{
					nextJob.OnDependencyExecuted(thread, *this);
				}
			}

			OnFinishedExecution(thread);
		}

		void SignalExecutionFinishedAndDestroying(JobRunnerThread& thread)
		{
			{
				while (m_nextJobs.HasElements())
				{
					RemoveSubsequentStage(m_nextJobs[0], thread, RemovalFlags::ExecuteIfDependenciesResolved);
				}
			}

			OnFinishedExecution(thread);
		}

		virtual void OnAwaitExternalFinish([[maybe_unused]] JobRunnerThread& thread)
		{
		}
	private:
		void OnDependenciesResolvedInternal(Threading::JobRunnerThread& thread)
		{
#if STAGE_DEPENDENCY_DEBUG
			{
				Threading::UniqueLock lock(m_dependencyLock);
				Assert(m_completedDependencies.GetSize() == m_dependencies.GetSize());
				m_completedDependencies.Clear();
				Assert(m_remainingDependencies.IsEmpty());
				m_remainingDependencies.CopyFrom(m_remainingDependencies.begin(), m_dependencies.GetView());
			}
#else
			Assert(AreAllDependenciesResolved());
			m_completedDependencyCount = 0;
#endif

			OnDependenciesResolved(thread);
		}
		virtual void OnDependenciesResolved(Threading::JobRunnerThread& thread)
		{
			SignalExecutionFinished(thread);
		}
		virtual void OnFinishedExecution([[maybe_unused]] JobRunnerThread& thread)
		{
		}
	private:
#if STAGE_DEPENDENCY_DEBUG
		mutable Threading::SharedMutex m_dependencyLock;
		Vector<ReferenceWrapper<StageBase>, DependencySizeType> m_dependencies;
		Vector<ReferenceWrapper<StageBase>, DependencySizeType> m_completedDependencies;
		Vector<ReferenceWrapper<StageBase>, DependencySizeType> m_remainingDependencies;
#else
		Atomic<DependencySizeType> m_dependencyCount = 0u;
		Atomic<DependencySizeType> m_completedDependencyCount = 0u;
#endif

		mutable Threading::SharedMutex m_nextJobMutex;
		Vector<ReferenceWrapper<StageBase>, DependencySizeType> m_nextJobs;
	};
}

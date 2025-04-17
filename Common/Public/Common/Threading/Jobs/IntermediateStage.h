#pragma once

#include "StageBase.h"

#include <Common/Threading/Mutexes/ConditionVariable.h>

#include <Common/Memory/Containers/StringView.h>

#if STAGE_DEPENDENCY_PROFILING
#include <Common/Memory/Containers/String.h>
#endif

namespace ngine::Threading
{
	struct IntermediateStage : public StageBase
	{
		IntermediateStage() = default;
		IntermediateStage([[maybe_unused]] const ConstStringView name)
#if STAGE_DEPENDENCY_PROFILING
			: m_name(name)
#endif
		{
		}

#if STAGE_DEPENDENCY_PROFILING
		[[nodiscard]] virtual ConstZeroTerminatedStringView GetDebugName() const override
		{
			return m_name;
		}
		String m_name;
#endif
	};

	struct ManualIntermediateStage : public StageBase
	{
		void Execute(JobRunnerThread& thread)
		{
			Assert(AreAllDependenciesResolved());
			SignalExecutionFinished(thread);
		}
	};

	struct DynamicIntermediateStage final : public IntermediateStage
	{
		using IntermediateStage::IntermediateStage;

		virtual void OnFinishedExecution(JobRunnerThread&) override
		{
			Assert(!HasSubsequentTasks());
			delete this;
		}

		virtual void OnDependenciesResolved(Threading::JobRunnerThread& thread) override
		{
			SignalExecutionFinishedAndDestroying(thread);
		}
	};

	[[nodiscard]] inline DynamicIntermediateStage& CreateIntermediateStage(const ConstStringView name = {})
	{
		return *new DynamicIntermediateStage(name);
	}
}

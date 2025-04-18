#pragma once

#include "Job.h"
#include "JobManager.h"
#include "CallbackResult.h"

#include <Common/Time/Duration.h>
#include <Common/TypeTraits/ReturnType.h>
#include <Common/TypeTraits/GetFunctionSignature.h>

#if PROFILE_BUILD
#include <Common/Memory/Containers/String.h>
#endif

namespace ngine::Threading
{
	struct AsyncCallbackJobBase : public Threading::Job
	{
		AsyncCallbackJobBase(const Priority priority, [[maybe_unused]] const ConstStringView name = {})
			: Job(priority)
#if STAGE_DEPENDENCY_PROFILING
			, m_name(name)
#endif
		{
		}
		virtual ~AsyncCallbackJobBase() = default;

#if STAGE_DEPENDENCY_PROFILING
		[[nodiscard]] virtual ConstZeroTerminatedStringView GetDebugName() const override
		{
			return m_name;
		}
		void SetDebugName(String&& name)
		{
			m_name = Forward<String>(name);
		}
#endif
	protected:
#if STAGE_DEPENDENCY_PROFILING
		String m_name;
#endif
	};

	template<typename Callback>
	struct AsyncCallbackJob final : public AsyncCallbackJobBase
	{
		AsyncCallbackJob(const Priority priority, Callback&& callback, [[maybe_unused]] const ConstStringView name = {})
			: AsyncCallbackJobBase(priority, name)
			, m_callback(Forward<Callback>(callback))
		{
		}
		virtual ~AsyncCallbackJob() = default;

		virtual Result OnExecute(Threading::JobRunnerThread& thread) override
		{
			if constexpr (TypeTraits::IsSame<TypeTraits::ReturnType<Callback>, void>)
			{
				m_callback(thread);
				return Job::Result::FinishedAndDelete;
			}
			else
			{
				return m_callback(thread);
			}
		}
	protected:
		Callback m_callback;
	};

	template<typename Callback>
	[[nodiscard]] inline AsyncCallbackJob<Callback>&
	CreateCallback(Callback&& callback, const JobPriority priority, [[maybe_unused]] const ConstStringView name = {})
	{
		return *new AsyncCallbackJob<Callback>(priority, Forward<Callback>(callback), name);
	}

	template<typename Callback, typename>
	inline void StageBase::AddSubsequentStage(Callback&& callback, const JobPriority priority, [[maybe_unused]] const ConstStringView name)
	{
		return StageBase::AddSubsequentStage(CreateCallback(Forward<Callback>(callback), priority, name));
	}

	template<typename Callback>
	inline TimerHandle JobManager::ScheduleAsync(const Time::Durationf delay, Callback&& callback, const JobPriority priority)
	{
		return ScheduleAsyncJob(delay, *new AsyncCallbackJob(priority, Forward<Callback>(callback)));
	}
}

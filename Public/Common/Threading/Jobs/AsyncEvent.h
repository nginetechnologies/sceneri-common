#pragma once

#include <Common/Memory/CachedFunctionInvocationQueue.h>

namespace ngine::Threading
{
	template<size ExtraSize, typename SignatureType>
	struct AsyncEvent;

	template<size ExtraSize, typename... ArgumentTypes>
	struct AsyncEvent<ExtraSize, void(ArgumentTypes...)>
	{
		struct AsyncJob : public Threading::Job
		{
			template<typename Function>
			AsyncJob(Function&& function) noexcept
				: m_function(Forward<Function>(function))
			{
			}

			virtual Result OnExecute(Threading::JobRunnerThread&) override
			{
				m_function();
				return Result::Finished;
			}
		protected:
			friend AsyncEvent;

			CachedFunctionInvocationQueue<ExtraSize, 1, ArgumentTypes...> m_function;
		};

		void Add(Threading::Job& job) noexcept
		{
			m_jobs.EmplaceBack(job);
		}

		using AsyncJobPtr = UniquePtr<AsyncJob>;

		template<typename Function>
		[[nodiscard]] AsyncJobPtr Add(Function&& function) noexcept
		{
			AsyncJobPtr pJob = AsyncJobPtr::Make(Forward<Function>(function));
			m_jobs.EmplaceBack(*pJob);
			return pJob;
		}

		void Remove(Threading::Job& job) noexcept
		{
			m_jobs.RemoveFirstOccurrence(job);
		}

		void operator()(Threading::JobManager& jobManager, ArgumentTypes&&... argumentTypes) noexcept
		{
			for (Threading::Job& job : m_jobs)
			{
				static_cast<AsyncJob&>(job).m_function.BindArguments(Forward<ArgumentTypes>(argumentTypes)...);
			}

			Threading::JobRunnerThread::GetCurrent().QueueJobsFromThread(m_jobs.GetView());
		}
	protected:
		Vector<ReferenceWrapper<Threading::Job>, uint16> m_jobs;
	};
}

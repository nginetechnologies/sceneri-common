#pragma once

namespace ngine::Threading
{
	struct Job;
	struct TimersJob;
	struct TimerData;

	struct TimerHandle
	{
		TimerHandle() = default;
		TimerHandle(Job& job, void* pHandle)
			: m_pJob(&job)
			, m_pHandle(pHandle)
		{
		}
		TimerHandle(TimerHandle&&);
		TimerHandle& operator=(TimerHandle&&);
		TimerHandle(const TimerHandle&);
		TimerHandle& operator=(const TimerHandle&);
		~TimerHandle();

		[[nodiscard]] bool IsValid() const
		{
			return m_pJob != nullptr;
		}

		[[nodiscard]] Optional<Job*> GetJob() const
		{
			return m_pJob;
		}
	protected:
		friend TimersJob;

		Optional<Job*> m_pJob;
		void* m_pHandle{nullptr};
	};
}

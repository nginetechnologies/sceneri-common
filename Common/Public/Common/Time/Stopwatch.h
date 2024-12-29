#pragma once

#include "Duration.h"

namespace ngine::Time
{
	struct Stopwatch
	{
		enum class StartType
		{
			Start
		};
		inline static constexpr StartType Elapsing = StartType::Start;

		using DurationType = Duration<double>;

		Stopwatch() = default;
		Stopwatch(StartType)
			: m_previousTime(DurationType::GetCurrentSystemUptime())
		{
		}

		void Start()
		{
			m_previousTime = DurationType::GetCurrentSystemUptime();
		}

		void Pause()
		{
			m_pauseTime = DurationType::GetCurrentSystemUptime();
		}

		void Resume()
		{
			m_previousTime += DurationType::GetCurrentSystemUptime() - m_pauseTime;
			m_pauseTime = 0_seconds;
		}

		void Stop()
		{
			m_previousTime = 0_seconds;
			m_pauseTime = 0_seconds;
		}

		[[nodiscard]] FORCE_INLINE bool IsRunning() const
		{
			return m_previousTime != 0_seconds && m_pauseTime == 0_seconds;
		}
		[[nodiscard]] FORCE_INLINE bool IsStopped() const
		{
			return m_previousTime == 0_seconds;
		}
		[[nodiscard]] FORCE_INLINE bool IsPaused() const
		{
			return m_pauseTime != 0_seconds;
		}

		[[nodiscard]] FORCE_INLINE DurationType GetElapsedTimeSince(const DurationType time) const
		{
			if (m_pauseTime != 0_seconds)
			{
				return (time - m_previousTime) - (time - m_pauseTime);
			}
			else
			{
				return time - m_previousTime;
			}
		}

		[[nodiscard]] FORCE_INLINE DurationType GetElapsedTime() const
		{
			return GetElapsedTimeSince(DurationType::GetCurrentSystemUptime());
		}

		[[nodiscard]] FORCE_INLINE DurationType GetElapsedTimeAndRestart()
		{
			const DurationType time = DurationType::GetCurrentSystemUptime();
			const DurationType delta = GetElapsedTimeSince(time);
			m_pauseTime = 0_seconds;
			m_previousTime = time;
			return delta;
		}

		FORCE_INLINE void Restart()
		{
			m_previousTime = DurationType::GetCurrentSystemUptime();
		}
	protected:
		DurationType m_previousTime = 0_seconds;
		DurationType m_pauseTime = 0_seconds;
	};
}

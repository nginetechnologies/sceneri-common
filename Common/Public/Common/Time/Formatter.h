#pragma once

#include <Common/Platform/ForceInline.h>

#include <Common/Time/Duration.h>

namespace ngine::Time
{
	struct Formatter
	{
		using PrecisionType = float;
		Formatter() = delete;

		template<typename DurationType>
		constexpr Formatter(const Duration<DurationType> duration)
			: m_value(static_cast<PrecisionType>(duration.GetSeconds()))
		{
		}

		[[nodiscard]] constexpr int64 GetHours() const
		{
			return (int64)(m_value / 3600.f) % 24;
		}

		[[nodiscard]] constexpr int64 GetMinutes() const
		{
			return (int64)(m_value / 60.f) % 60;
		}

		[[nodiscard]] constexpr int64 GetSeconds() const
		{
			return (int64)m_value % 60;
		}

		[[nodiscard]] constexpr int64 GetMilliseconds() const
		{
			return (int64)(m_value * 1000.f) % 1000;
		}

		[[nodiscard]] constexpr int64 GetNanoseconds() const
		{
			return (int64)((m_value * 1000.f) * 1000000) % 1000000;
		}
	protected:
		constexpr Formatter(const PrecisionType seconds)
			: m_value(seconds)
		{
		}
	protected:
		PrecisionType m_value;
	};
}

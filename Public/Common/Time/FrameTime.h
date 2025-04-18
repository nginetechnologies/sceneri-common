#pragma once

#include "Duration.h"

namespace ngine
{
	struct FrameTime
	{
		FrameTime() = default;

		explicit FrameTime(const Time::Durationd value)
			: m_value(value)
		{
		}

		[[nodiscard]] FORCE_INLINE operator Time::Durationd() const
		{
			return m_value;
		}
		[[nodiscard]] FORCE_INLINE operator Time::Durationf() const
		{
			return Time::Durationf::FromSeconds((float)m_value.GetSeconds());
		}

		[[nodiscard]] FORCE_INLINE operator float() const
		{
			return (float)m_value.GetSeconds();
		}
	protected:
		Time::Durationd m_value;
	};
}

#pragma once

#include "Duration.h"

namespace ngine
{
	struct FrameTime
	{
		using PrecisionType = float;

		FrameTime() = default;

		template<typename OtherPrecisionType>
		explicit FrameTime(const Time::Duration<OtherPrecisionType> value)
			: m_value(static_cast<PrecisionType>(value.GetSeconds()))
		{
		}

		[[nodiscard]] FORCE_INLINE operator Time::Durationf() const
		{
			return Time::Durationf::FromSeconds(m_value);
		}
	public:
		[[nodiscard]] FORCE_INLINE operator PrecisionType() const
		{
			return m_value;
		}
	protected:
		PrecisionType m_value;
	};
}

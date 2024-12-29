#pragma once

#include <Common/Math/Clamp.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr T SmoothStep(const T min, const T max, T value) noexcept
	{
		value = Math::Saturate((value - min) / (max - min));
		return value * value * (3 - 2 * value);
	}
}

#pragma once

#include <Common/Math/Select.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/Math/MathAssert.h>

#include <math.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Log10(const float value) noexcept
	{
		return ::log10f(value);
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Log10(const double value) noexcept
	{
		return ::log10(value);
	}
}

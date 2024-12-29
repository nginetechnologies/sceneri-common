#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr bool IsNegative(const int32 value) noexcept
	{
		return static_cast<uint32>(value) >> 31;
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr bool IsNegative(const float value) noexcept
	{
		return value < 0.f;
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr bool IsNegative(const double value) noexcept
	{
		return value < 0;
	}
}

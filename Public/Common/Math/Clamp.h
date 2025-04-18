#pragma once

#include <Common/Math/Min.h>
#include <Common/Math/Max.h>
#include <Common/Platform/IsConstantEvaluated.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr T Clamp(const T value, const T min, const T max) noexcept
	{
		return Min(Max(value, min), max);
	}

#if USE_SSE
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr float Clamp(float value, const float min, const float max) noexcept
	{
		if (IsConstantEvaluated())
		{
			return (value < min) ? min : ((value > max) ? max : value);
		}
		else
		{
			_mm_store_ss(&value, _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(min)), _mm_set_ss(max)));
			return value;
		}
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr double Clamp(double value, const double min, const double max) noexcept
	{
		if (IsConstantEvaluated())
		{
			return (value < min) ? min : ((value > max) ? max : value);
		}
		else
		{
			_mm_store_sd(&value, _mm_min_sd(_mm_max_sd(_mm_set_sd(value), _mm_set_sd(min)), _mm_set_sd(max)));
			return value;
		}
	}
#endif

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr T Saturate(const T value) noexcept
	{
		return Clamp(value, T(0), T(1));
	}
}

#pragma once

#include <Common/Math/Select.h>
#include <Common/Platform/IsConstantEvaluated.h>

#if USE_SSE
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

namespace ngine::Math
{
	template<typename T1, typename T2>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr auto Max(const T1 a, const T2 b) noexcept
	{
		return Math::Select(a > b, a, b);
	}

	template<typename T0, typename T1, typename... Ts>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr auto Max(T0 val1, T1 val2, Ts... vs)
	{
		return Max(Max(val1, val2), vs...);
	}

#if USE_SSE
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr float floatMax(const float a, const float b) noexcept
	{
		if (IsConstantEvaluated())
		{
			return a > b ? a : b;
		}
		else
		{
			return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
		}
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr double Max(const double a, const double b) noexcept
	{
		if (IsConstantEvaluated())
		{
			return a > b ? a : b;
		}
		else
		{
			return _mm_cvtsd_f64(_mm_max_sd(_mm_set_sd(a), _mm_set_sd(b)));
		}
	}
#endif

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T* Max(T* const a, T* const b) noexcept
	{
		return reinterpret_cast<T*>(Max(reinterpret_cast<uintptr>(a), reinterpret_cast<uintptr>(b)));
	}
}

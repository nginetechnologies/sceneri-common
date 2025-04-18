#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsSigned.h>

#include <cmath>

#if USE_SSE
#include <xmmintrin.h>
#endif

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr T SignNonZero(const T value) noexcept
	{
		if constexpr (TypeTraits::IsUnsigned<T>)
		{
			return 1;
		}
		else
		{
			const T sign = (value >> (sizeof(T) * 8 - 1)) + 1;
			return T(-1) + sign * T(2);
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float SignNonZero(const float value) noexcept
	{
#if USE_SSE
		__m128 negzero = _mm_set_ss(-0.0f);
		__m128 x_signbit = _mm_and_ps(_mm_set_ss(value), negzero);
		return _mm_cvtss_f32(_mm_or_ps(_mm_set_ss(1.0f), x_signbit));
#else
		union
		{
			float f;
			int32 i;
		} x;
		x.f = value;
		return (float)SignNonZero(x.i);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double SignNonZero(const double value) noexcept
	{
#if USE_SSE
		__m128d negzero = _mm_set_sd(-0.0);
		__m128d x_signbit = _mm_and_pd(_mm_set_sd(value), negzero);
		return _mm_cvtsd_f64(_mm_or_pd(_mm_set_sd(1.0), x_signbit));
#else
		union
		{
			double f;
			int64 i;
		} x;
		x.f = value;
		return (double)SignNonZero(x.i);
#endif
	}
}

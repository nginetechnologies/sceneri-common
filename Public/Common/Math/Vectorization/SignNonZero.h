#pragma once

#include "Packed.h"

#include <Common/Math/SignNonZero.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> SignNonZero(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SSE
		__m128 negzero = _mm_set1_ps(-0.0f);
		__m128 x_signbit = _mm_and_ps(value, negzero);
		return _mm_or_ps(_mm_set1_ps(1.0f), x_signbit);
#else
		return {SignNonZero(value[0]), SignNonZero(value[1]), SignNonZero(value[2]), SignNonZero(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> SignNonZero(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SSE
		__m128d negzero = _mm_set1_pd(-0.0);
		__m128d x_signbit = _mm_and_pd(value, negzero);
		return _mm_or_pd(_mm_set1_pd(1.0), x_signbit);
#else
		return {SignNonZero(value[0]), SignNonZero(value[1])};
#endif
	}
}

#pragma once

#include "Packed.h"

#include <Common/Math/Sign.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Sign(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SSE
		__m128 negzero = _mm_set1_ps(-0.0f);
		__m128 nonzero = _mm_cmpneq_ps(value, _mm_setzero_ps());
		__m128 x_signbit = _mm_and_ps(value, negzero);
		__m128 zeroone = _mm_and_ps(nonzero, _mm_set1_ps(1.0f));
		return _mm_or_ps(zeroone, x_signbit);
#else
		return {Sign(value[0]), Sign(value[1]), Sign(value[2]), Sign(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Sign(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SSE
		__m128d negzero = _mm_set1_pd(-0.0);
		__m128d nonzero = _mm_cmpneq_pd(value, _mm_setzero_pd());
		__m128d x_signbit = _mm_and_pd(value, negzero);
		__m128d zeroone = _mm_and_pd(nonzero, _mm_set1_pd(1.0));
		return _mm_or_pd(zeroone, x_signbit);
#else
		return {Sign(value[0]), Sign(value[1])};
#endif
	}
}

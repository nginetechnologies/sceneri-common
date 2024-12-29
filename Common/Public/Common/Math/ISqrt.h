#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Math/Sqrt.h>

#if USE_SSE
#include <xmmintrin.h>
#include <emmintrin.h>
#else
#include <math.h>
#endif

#if USE_AVX512
#include <immintrin.h>
#else
#include <Common/Math/MultiplicativeInverse.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Isqrt(const float value) noexcept
	{
#if USE_SSE
		return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(value)));
#else
		return MultiplicativeInverse(Sqrt(value));
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Isqrt(const double value) noexcept
	{
#if USE_AVX512
		__m128d result = {0, 0};
		return _mm_cvtsd_f64(_mm_rsqrt28_sd(result, _mm_set_sd(value)));
#else
		return MultiplicativeInverse(Sqrt(value));
#endif
	}
}

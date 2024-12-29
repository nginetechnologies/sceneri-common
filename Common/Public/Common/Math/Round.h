#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>

#if USE_SSE4_1
#include <smmintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Round(const float value) noexcept
	{
#if USE_SSE4_1
		__m128 result{0};
		return _mm_cvtss_f32(_mm_round_ss(result, _mm_set_ss(value), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
#else
		return ::roundf(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Round(const double value) noexcept
	{
#if USE_SSE4_1
		__m128d result{0};
		return _mm_cvtsd_f64(_mm_round_sd(result, _mm_set_sd(value), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
#else
		return ::round(value);
#endif
	}
}

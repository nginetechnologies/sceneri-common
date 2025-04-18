#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Sin(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_sin_pd(_mm_set_sd(value)));
#else
		return ::sin(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Sin(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(value)));
#else
		return ::sinf(value);
#endif
	}
}

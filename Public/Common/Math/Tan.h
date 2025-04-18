#pragma once

#include <Common/Platform/ForceInline.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Tan(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_tan_pd(_mm_set_sd(value)));
#else
		return ::tan(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Tan(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(value)));
#else
		return ::tanf(value);
#endif
	}
}

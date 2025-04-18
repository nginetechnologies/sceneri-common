#pragma once

#include <Common/Platform/ForceInline.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Atan2(const double y, const double x) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_atan2_pd(_mm_set_sd(y), _mm_set_sd(x)));
#else
		return ::atan2(y, x);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Atan2(const float y, const float x) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(y), _mm_set_ss(x)));
#else
		return ::atan2f(y, x);
#endif
	}
}

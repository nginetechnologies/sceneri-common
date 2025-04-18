#pragma once

#include <Common/Platform/ForceInline.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Atan(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_atan_pd(_mm_set_sd(value)));
#else
		return ::atan(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Atan(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_atan_ps(_mm_set_ss(value)));
#else
		return ::atanf(value);
#endif
	}
}

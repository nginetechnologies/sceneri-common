#pragma once

#include <Common/Platform/ForceInline.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Cos(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_cos_pd(_mm_set_sd(value)));
#else
		return ::cos(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Cos(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_cos_ps(_mm_set_ss(value)));
#else
		return ::cosf(value);
#endif
	}
}

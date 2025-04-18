#pragma once

#include <Common/Platform/ForceInline.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE double Acos(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_acos_pd(_mm_set_sd(value)));
#else
		return ::acos(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE float Acos(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_acos_ps(_mm_set_ss(value)));
#else
		return ::acosf(value);
#endif
	}
}

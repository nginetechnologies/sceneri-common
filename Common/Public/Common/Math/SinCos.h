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
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double SinCos(const double value, double& cosOut) noexcept
	{
#if USE_SVML
		__m128d cos;
		const double sin = _mm_cvtsd_f64(_mm_sincos_pd(&cos, _mm_set_sd(value)));
		cosOut = _mm_cvtsd_f64(cos);
		return sin;
#else
		cosOut = ::cos(value);
		return ::sin(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float SinCos(const float value, float& cosOut) noexcept
	{
#if USE_SVML
		__m128 cos;
		const float sin = _mm_cvtss_f32(_mm_sincos_ps(&cos, _mm_set_ss(value)));
		cosOut = _mm_cvtss_f32(cos);
		return sin;
#else
		cosOut = ::cosf(value);
		return ::sinf(value);
#endif
	}
}

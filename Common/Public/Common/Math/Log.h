#pragma once

#include <Common/Math/Select.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/Math/MathAssert.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Log(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_log_ps(_mm_set_ss(value)));
#else
		return ::logf(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Log(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_log_pd(_mm_set_sd(value)));
#else
		return ::log(value);
#endif
	}
}

#pragma once

#include <Common/Platform/ForceInline.h>

#define HAS_CBRT_INTRINSIC USE_SSE && !PLATFORM_APPLE && !COMPILER_CLANG_WINDOWS && !PLATFORM_LINUX && !PLATFORM_WEB

#if HAS_CBRT_INTRINSIC
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
#if HAS_CBRT_INTRINSIC
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double CubicRoot(const double value) noexcept
	{
		__m128d storedValue = _mm_set_sd(value);
		return _mm_cvtsd_f64(_mm_cbrt_pd(storedValue));
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float CubicRoot(const float value) noexcept
	{
		return _mm_cvtss_f32(_mm_cbrt_ps(_mm_set_ss(value)));
	}
#else
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double CubicRoot(const double value) noexcept
	{
		return ::cbrt(value);
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float CubicRoot(const float value) noexcept
	{
		return ::cbrtf(value);
	}
#endif
}

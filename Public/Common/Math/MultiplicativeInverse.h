#pragma once

#if USE_SSE
#include <xmmintrin.h>
#endif

namespace ngine::Math
{
#if USE_SSE
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float MultiplicativeInverse(const float value) noexcept
	{
		return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(value)));
	}
#else
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float MultiplicativeInverse(const float value) noexcept
	{
		return 1.f / value;
	}
#endif

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double MultiplicativeInverse(const double value) noexcept
	{
		return 1.0 / value;
	}
}

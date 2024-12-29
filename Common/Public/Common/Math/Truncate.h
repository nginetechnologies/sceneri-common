#pragma once

#if USE_SSE
#include <xmmintrin.h>
#include <emmintrin.h>
#else
#include <math.h>
#endif

#if USE_SSE4_1
#include <smmintrin.h>
#endif

namespace ngine::Math
{
	template<typename ToType, typename FromType, typename ReturnType = ToType>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS ReturnType Truncate(const FromType value) noexcept = delete;

	template<>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int32 Truncate<int32>(const float value) noexcept
	{
#if USE_SSE
		return _mm_cvttss_si32(_mm_set_ss(value));
#else
		return (int32)value;
#endif
	}
	template<>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int64 Truncate<int64>(const float value) noexcept
	{
#if USE_SSE
		return _mm_cvttss_si64(_mm_set_ss(value));
#else
		return (int64)value;
#endif
	}

	template<>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int32 Truncate<int32>(const double value) noexcept
	{
#if USE_SSE
		return _mm_cvttsd_si32(_mm_set_sd(value));
#else
		return (int32)value;
#endif
	}
	template<>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int64 Truncate<int64>(const double value) noexcept
	{
#if USE_SSE
		return _mm_cvttsd_si64(_mm_set_sd(value));
#else
		return (int64)value;
#endif
	}
}

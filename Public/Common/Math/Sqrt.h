#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/Math/Round.h>
#include <Common/Math/CoreNumericTypes.h>

#if USE_SSE
#include <xmmintrin.h>
#include <emmintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Sqrt(const double value) noexcept
	{
#if USE_SSE
		__m128d storedValue = _mm_set_sd(value);
		return _mm_cvtsd_f64(_mm_sqrt_sd(storedValue, storedValue));
#else
		return ::sqrt(value);
#endif
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Sqrt(const float value) noexcept
	{
#if USE_SSE
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
#else
		return ::sqrtf(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint32 Sqrt(const uint32 value) noexcept
	{
		return (uint32)Math::Round(Math::Sqrt((float)value));
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint64 Sqrt(const uint64 value) noexcept
	{
		return (uint64)Math::Round(Math::Sqrt((double)value));
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int32 Sqrt(const int32 value) noexcept
	{
		return (int32)Math::Round(Math::Sqrt((float)value));
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int64 Sqrt(const int64 value) noexcept
	{
		return (int64)Math::Round(Math::Sqrt((double)value));
	}
}

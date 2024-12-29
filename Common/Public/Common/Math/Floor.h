#pragma once

#if USE_SSE4_1
#include <smmintrin.h>
#else
#include <math.h>
#endif

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Floor(const float value) noexcept
	{
#if USE_SSE4_1
		__m128 result{0};
		return _mm_cvtss_f32(_mm_floor_ss(result, _mm_set_ss(value)));
#else
		return ::floorf(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Floor(const double value) noexcept
	{
#if USE_SSE4_1
		__m128d result{0};
		return _mm_cvtsd_f64(_mm_floor_sd(result, _mm_set_sd(value)));
#else
		return ::floor(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint8 Floor(const uint8 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint16 Floor(const uint16 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint32 Floor(const uint32 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS unsigned long Floor(const unsigned long value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS unsigned long long Floor(const unsigned long long value) noexcept
	{
		return value;
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int8 Floor(const int8 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int16 Floor(const int16 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int32 Floor(const int32 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS signed long Floor(const signed long value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS signed long long Floor(const signed long long value) noexcept
	{
		return value;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T* Floor(const T* value) noexcept
	{
		return value;
	}
}

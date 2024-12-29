#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsSigned.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr EnableIf<TypeTraits::IsUnsigned<T>, T> Sign(const T value) noexcept
	{
		return T(T(0) < value);
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr EnableIf<TypeTraits::IsSigned<T>, T> Sign(const T value) noexcept
	{
		return T(T(0) < value) - (value < T(0));
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Sign(const float value) noexcept
	{
#if USE_SSE
		__m128 x = _mm_set_ss(value);
		__m128 negzero = _mm_set_ss(-0.0f);
		__m128 nonzero = _mm_cmpneq_ps(x, _mm_setzero_ps());
		__m128 x_signbit = _mm_and_ps(x, negzero);
		__m128 zeroone = _mm_and_ps(nonzero, _mm_set_ss(1.0f));
		return _mm_cvtss_f32(_mm_or_ps(zeroone, x_signbit));
#else
		return (float)Sign((int)value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Sign(const double value) noexcept
	{
#if USE_SSE
		__m128d x = _mm_set_sd(value);
		__m128d negzero = _mm_set_sd(-0.0);
		__m128d nonzero = _mm_cmpneq_pd(x, _mm_setzero_pd());
		__m128d x_signbit = _mm_and_pd(x, negzero);
		__m128d zeroone = _mm_and_pd(nonzero, _mm_set_sd(1.0));
		return _mm_cvtsd_f64(_mm_or_pd(zeroone, x_signbit));
#else
		return (double)Sign((int64)value);
#endif
	}
}

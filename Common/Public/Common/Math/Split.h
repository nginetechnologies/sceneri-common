#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>

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
	template<typename T>
	struct SplitResult
	{
		T m_integer;
		T m_fraction;
	};

	using SplitResultd = SplitResult<double>;
	using SplitResultf = SplitResult<float>;

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS SplitResultd Split(const double value) noexcept
	{
		SplitResultd result;
#if USE_SSE
		__m128d valuePacked = _mm_set_sd(value);
		__m128d integer = _mm_round_pd(valuePacked, _MM_FROUND_TRUNC);
		result.m_integer = _mm_cvtsd_f64(integer);
		result.m_fraction = _mm_cvtsd_f64(_mm_sub_pd(valuePacked, integer));
#else
		result.m_fraction = ::modf(value, &result.m_integer);
#endif
		return result;
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS SplitResultf Split(const float value) noexcept
	{
		SplitResultf result;
#if USE_SSE
		__m128 valuePacked = _mm_set_ss(value);
		__m128 integer = _mm_round_ps(valuePacked, _MM_FROUND_TRUNC);
		result.m_integer = _mm_cvtss_f32(integer);
		result.m_fraction = _mm_cvtss_f32(_mm_sub_ps(valuePacked, integer));
#else
		result.m_fraction = ::modf(value, &result.m_integer);
#endif
		return result;
	}
}

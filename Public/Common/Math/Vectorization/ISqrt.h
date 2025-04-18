#pragma once

#include "Packed.h"

#include <Common/Math/ISqrt.h>
#include <Common/Math/Vectorization/MultiplicativeInverse.h>
#include <Common/Math/Vectorization/Sqrt.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Isqrt(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SSE
		return _mm_rsqrt_ps(value.m_value);
#else
		return Math::MultiplicativeInverse(Sqrt(value));
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Isqrt(const Vectorization::Packed<double, 2> value) noexcept
	{
		return Math::MultiplicativeInverse(Sqrt(value));
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Isqrt(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_rsqrt_ps(value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Isqrt(const Vectorization::Packed<double, 4> value) noexcept
	{
		return Math::MultiplicativeInverse(Sqrt(value));
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Isqrt(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_rsqrt14_ps(value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Isqrt(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_rsqrt14_pd(value.m_value);
	}
#endif
}

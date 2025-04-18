#pragma once

#include "Packed.h"

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> MultiplicativeInverse(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SSE
		return _mm_rcp_ps(value);
#elif USE_NEON
		float32x4_t approx = vrecpeq_f32(value);
		return vmulq_f32(approx, vrecpsq_f32(value, approx));
#else
		using PackedType = Vectorization::Packed<float, 4>;
		return PackedType(1.0f) / value;
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> MultiplicativeInverse(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_AVX512
		return _mm_rcp14_pd(value);
#else
		using PackedType = Vectorization::Packed<double, 2>;
		return PackedType(1.0) / value;
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> MultiplicativeInverse(const Vectorization::Packed<float, 8> value) noexcept
	{
#if USE_AVX
		return _mm256_rcp_ps(value);
#elif USE_AVX512
		return _mm256_rcp14_ps(value);
#else
		using PackedType = Vectorization::Packed<float, 8>;
		return PackedType(1.f) / value;
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> MultiplicativeInverse(const Vectorization::Packed<double, 4> value) noexcept
	{
#if USE_AVX512
		return _mm256_rcp14_pd(value);
#else
		using PackedType = Vectorization::Packed<double, 4>;
		return PackedType(1.0) / value;
#endif
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> MultiplicativeInverse(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_rcp14_ps(value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> MultiplicativeInverse(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_rcp14_pd(value);
	}
#endif
}

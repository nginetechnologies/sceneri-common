#pragma once

#include "Packed.h"

#include <Common/Math/Ceil.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Ceil(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f32x4_ceil(value.m_value);
#elif USE_SSE4_1
		return _mm_ceil_ps(value);
#else
		return {Ceil(value[0]), Ceil(value[1]), Ceil(value[2]), Ceil(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Ceil(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f64x2_ceil(value.m_value);
#elif USE_SSE4_1
		return _mm_ceil_pd(value);
#else
		return {Ceil(value[0]), Ceil(value[1])};
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Ceil(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_ceil_ps(value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Ceil(const Vectorization::Packed<double, 4> value) noexcept
	{
		return _mm256_ceil_pd(value);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Ceil(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_ceil_ps(value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Ceil(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_ceil_pd(value);
	}
#endif
}

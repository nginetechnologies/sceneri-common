#pragma once

#include "Packed.h"

#include <Common/Math/Floor.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Floor(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f32x4_floor(value.m_value);
#elif USE_SSE4_1
		return _mm_floor_ps(value);
#else
		return {Floor(value[0]), Floor(value[1]), Floor(value[2]), Floor(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Floor(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f64x2_floor(value.m_value);
#elif USE_SSE4_1
		return _mm_floor_pd(value);
#else
		return {Floor(value[0]), Floor(value[1])};
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Floor(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_floor_ps(value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Floor(const Vectorization::Packed<double, 4> value) noexcept
	{
		return _mm256_floor_pd(value);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Floor(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_floor_ps(value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Floor(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_floor_pd(value);
	}
#endif
}

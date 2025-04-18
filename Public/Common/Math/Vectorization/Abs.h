#pragma once

#include "Packed.h"

#include <Common/Math/Abs.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Abs(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f32x4_abs(value.m_value);
#elif USE_SSE
		return _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), value.m_value), value.m_value);
#elif USE_NEON
		return vabsq_f32(value.m_value);
#else
		return {Abs(value[0]), Abs(value[1]), Abs(value[2]), Abs(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Abs(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f64x2_abs(value.m_value);
#elif USE_SSE
		return _mm_max_pd(_mm_sub_pd(_mm_setzero_pd(), value.m_value), value.m_value);
#elif USE_NEON
		return vabsq_f64(value.m_value);
#else
		return {Abs(value[0]), Abs(value[1]), Abs(value[2]), Abs(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<int32, 4> Abs(const Vectorization::Packed<int32, 4> value) noexcept
	{
		return {Abs(value[0]), Abs(value[1]), Abs(value[2]), Abs(value[3])};
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Abs(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_andnot_ps(_mm256_set1_ps(-0.f), value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Abs(const Vectorization::Packed<double, 4> value) noexcept
	{
		return _mm256_andnot_pd(_mm256_set1_pd(-0.), value.m_value);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Abs(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_andnot_ps(_mm512_set1_ps(-0.f), value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Abs(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_andnot_pd(_mm512_set1_pd(-0.), value.m_value);
	}
#endif
}

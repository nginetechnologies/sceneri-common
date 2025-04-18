#pragma once

#include "Packed.h"

#include <Common/Math/Round.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Round(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f32x4_nearest(value.m_value);
#elif USE_SSE4_1
		return _mm_round_ps(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#else
		return {Round(value[0]), Round(value[1]), Round(value[2]), Round(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Round(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f64x2_nearest(value.m_value);
#elif USE_SSE4_1
		return _mm_round_pd(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#else
		return {Round(value[0]), Round(value[1])};
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Round(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_round_ps(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Round(const Vectorization::Packed<double, 4> value) noexcept
	{
		return _mm256_round_pd(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Round(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_round_ps(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Round(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_round_pd(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	}
#endif
}

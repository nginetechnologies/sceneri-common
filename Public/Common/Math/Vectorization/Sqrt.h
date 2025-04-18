#pragma once

#include "Packed.h"

#include <Common/Math/Sqrt.h>
#include <Common/Math/Vectorization/MultiplicativeInverse.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Sqrt(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f32x4_sqrt(value.m_value);
#elif USE_SSE
		return _mm_sqrt_ps(value.m_value);
#elif USE_NEON
		return vsqrtq_f32(value.m_value);
#else
		return {Sqrt(mF32[0]), Sqrt(mF32[1]), Sqrt(mF32[2]), Sqrt(mF32[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Sqrt(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f64x2_sqrt(value.m_value);
#elif USE_SSE
		return _mm_sqrt_pd(value.m_value);
#elif USE_NEON
		return vsqrtq_f64(value.m_value);
#else
		return {Sqrt(mF32[0]), Sqrt(mF32[1]), Sqrt(mF32[2]), Sqrt(mF32[3])};
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Sqrt(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_sqrt_ps(value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Sqrt(const Vectorization::Packed<double, 4> value) noexcept
	{
		return _mm256_sqrt_pd(value.m_value);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Sqrt(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_sqrt_ps(value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Sqrt(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_sqrt_pd(value.m_value);
	}
#endif
}

#pragma once

#include "Packed.h"

#include <Common/Math/Min.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4>
	Min(const Vectorization::Packed<float, 4> a, const Vectorization::Packed<float, 4> b) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f32x4_min(a, b);
#elif USE_SSE
		return _mm_min_ps(a, b);
#elif USE_NEON
		return vminq_f32(a, b);
#else
		return {Min(a[0], b[0]), Min(a[1], b[1]), Min(a[2], b[2]), Min(a[3], b[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2>
	Min(const Vectorization::Packed<double, 2> a, const Vectorization::Packed<double, 2> b) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_f64x2_min(a, b);
#elif USE_SSE
		return _mm_min_pd(a, b);
#elif USE_NEON
		return vminq_f64(a, b);
#else
		return {Min(a[0], b[0]), Min(a[1], b[1])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<int32, 4>
	Min(const Vectorization::Packed<int32, 4> a, const Vectorization::Packed<int32, 4> b) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_i32x4_min(a, b);
#elif USE_SSE4_1
		return _mm_min_epi32(a, b);
#elif USE_NEON
		return vminq_s32(a, b);
#else
		return {Min(a[0], b[0]), Min(a[1], b[1]), Min(a[2], b[2]), Min(a[3], b[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<uint32, 4>
	Min(const Vectorization::Packed<uint32, 4> a, const Vectorization::Packed<uint32, 4> b) noexcept
	{
#if USE_WASM_SIMD128
		return wasm_u32x4_min(a, b);
#elif USE_SSE4_1
		return _mm_min_epu32(a, b);
#elif USE_NEON
		return vminq_u32(a, b);
#else
		return {Min(a[0], b[0]), Min(a[1], b[1]), Min(a[2], b[2]), Min(a[3], b[3])};
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8>
	Min(const Vectorization::Packed<float, 8> a, const Vectorization::Packed<float, 8> b) noexcept
	{
		return _mm256_min_ps(a, b);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4>
	Min(const Vectorization::Packed<double, 4> a, const Vectorization::Packed<double, 4> b) noexcept
	{
		return _mm256_min_pd(a, b);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16>
	Min(const Vectorization::Packed<float, 16> a, const Vectorization::Packed<float, 16> b) noexcept
	{
		return _mm512_min_ps(a, b);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8>
	Min(const Vectorization::Packed<double, 8> a, const Vectorization::Packed<double, 8> b) noexcept
	{
		return _mm512_min_pd(a, b);
	}
#endif
}

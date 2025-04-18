#pragma once

#include "Packed.h"

#include <Common/Math/CubicRoot.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> CubicRoot(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_cbrt_ps(value.m_value);
#else
		return {Math::CubicRoot(value[0]), Math::CubicRoot(value[1]), Math::CubicRoot(value[2]), Math::CubicRoot(value[3])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> CubicRoot(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_cbrt_pd(value.m_value);
#else
		return {Math::CubicRoot(value[0]), Math::CubicRoot(value[1])};
#endif
	}

#if USE_AVX && USE_SVML
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> CubicRoot(const Vectorization::Packed<float, 8> value) noexcept
	{
		return _mm256_cbrt_ps(value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> CubicRoot(const Vectorization::Packed<double, 4> value) noexcept
	{
		return _mm256_cbrt_pd(value.m_value);
	}
#endif

#if USE_AVX512 && USE_SVML
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> CubicRoot(const Vectorization::Packed<float, 16> value) noexcept
	{
		return _mm512_cbrt_ps(value.m_value);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> CubicRoot(const Vectorization::Packed<double, 8> value) noexcept
	{
		return _mm512_cbrt_pd(value.m_value);
	}
#endif
}

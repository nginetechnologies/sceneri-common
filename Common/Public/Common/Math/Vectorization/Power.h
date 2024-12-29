#pragma once

#include "Packed.h"
#include <Common/Math/Power.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4>
	Power(const Vectorization::Packed<float, 4> base, const Vectorization::Packed<float, 4> exponent) noexcept
	{
#if USE_SVML
		return _mm_pow_ps(base.m_value, exponent.m_value);
#else
		return {
			Math::Power(base[0], exponent[0]),
			Math::Power(base[1], exponent[1]),
			Math::Power(base[2], exponent[2]),
			Math::Power(base[3], exponent[3])
		};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2>
	Power(const Vectorization::Packed<double, 2> base, const Vectorization::Packed<double, 2> exponent) noexcept
	{
#if USE_SVML
		return _mm_pow_pd(base.m_value, exponent.m_value);
#else
		return {Math::Power(base[0], exponent[0]), Math::Power(base[1], exponent[1])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Power2(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Math::Power2(value[0]), Math::Power2(value[1]), Math::Power2(value[2]), Math::Power2(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Power2(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Math::Power2(value[0]), Math::Power2(value[1])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Power10(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Math::Power10(value[0]), Math::Power10(value[1]), Math::Power10(value[2]), Math::Power10(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Power10(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Math::Power10(value[0]), Math::Power10(value[1])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Exponential(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Math::Exponential(value[0]), Math::Exponential(value[1]), Math::Exponential(value[2]), Math::Exponential(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Exponential(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Math::Exponential(value[0]), Math::Exponential(value[1])};
	}
}

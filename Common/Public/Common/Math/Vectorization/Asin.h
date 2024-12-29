#pragma once

#include "Packed.h"
#include <Common/Math/Asin.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Asin(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_asin_ps(value.m_value);
#else
		return {Math::Asin(value[0]), Math::Asin(value[1]), Math::Asin(value[2]), Math::Asin(value[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Asin(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_asin_pd(value.m_value);
#else
		return {Math::Asin(value[0]), Math::Asin(value[1])};
#endif
	}
}

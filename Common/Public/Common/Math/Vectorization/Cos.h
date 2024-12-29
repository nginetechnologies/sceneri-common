#pragma once

#include "Packed.h"
#include <Common/Math/Cos.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Cos(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_cos_ps(value.m_value);
#else
		return {Math::Cos(value[0]), Math::Cos(value[1]), Math::Cos(value[2]), Math::Cos(value[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Cos(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_cos_pd(value.m_value);
#else
		return {Math::Cos(value[0]), Math::Cos(value[1])};
#endif
	}
}

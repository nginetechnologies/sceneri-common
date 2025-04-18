#pragma once

#include "Packed.h"
#include <Common/Math/Sin.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Sin(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_sin_ps(value.m_value);
#else
		return {Sin(value[0]), Sin(value[1]), Sin(value[2]), Sin(value[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Sin(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_sin_pd(value.m_value);
#else
		return {Sin(value[0]), Sin(value[1])};
#endif
	}
}

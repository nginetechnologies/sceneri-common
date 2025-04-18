#pragma once

#include "Packed.h"
#include <Common/Math/Atan.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Atan(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_atan_ps(value.m_value);
#else
		return {Math::Atan(value[0]), Math::Atan(value[1]), Math::Atan(value[2]), Math::Atan(value[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Atan(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_atan_pd(value.m_value);
#else
		return {Math::Atan(value[0]), Math::Atan(value[1])};
#endif
	}
}

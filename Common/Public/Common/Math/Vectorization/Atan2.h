#pragma once

#include "Packed.h"
#include <Common/Math/Atan2.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4>
	Atan2(const Vectorization::Packed<float, 4> a, const Vectorization::Packed<float, 4> b) noexcept
	{
#if USE_SVML
		return _mm_atan2_ps(a.m_value, b.m_value);
#else
		return {Math::Atan2(a[0], b[0]), Math::Atan2(a[1], b[1]), Math::Atan2(a[2], b[2]), Math::Atan2(a[3], b[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2>
	Atan2(const Vectorization::Packed<double, 2> a, const Vectorization::Packed<double, 2> b) noexcept
	{
#if USE_SVML
		return _mm_atan2_pd(a.m_value, b.m_value);
#else
		return {Math::Atan2(a[0], b[0]), Math::Atan2(a[1], b[1])};
#endif
	}
}

#pragma once

#include "Packed.h"
#include <Common/Math/Tan.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Tan(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_tan_ps(value.m_value);
#else
		return {Tan(value[0]), Tan(value[1]), Tan(value[2]), Tan(value[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Tan(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_tan_pd(value.m_value);
#else
		return {Tan(value[0]), Tan(value[1])};
#endif
	}
}

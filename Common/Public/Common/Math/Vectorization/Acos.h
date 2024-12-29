#pragma once

#include "Packed.h"
#include <Common/Math/Acos.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Acos(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SVML
		return _mm_acos_ps(value.m_value);
#else
		return {Math::Acos(value[0]), Math::Acos(value[1]), Math::Acos(value[2]), Math::Acos(value[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Acos(const Vectorization::Packed<double, 2> value) noexcept
	{
#if USE_SVML
		return _mm_acos_pd(value.m_value);
#else
		return {Math::Acos(value[0]), Math::Acos(value[1])};
#endif
	}
}

#pragma once

#include "Packed.h"
#include <Common/Math/SinCos.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4>
	SinCos(const Vectorization::Packed<float, 4> value, Vectorization::Packed<float, 4>& cosOut) noexcept
	{
#if USE_SVML
		return _mm_sincos_ps(&cosOut.m_value, value.m_value);
#else
		return {SinCos(value[0], cosOut[0]), SinCos(value[1], cosOut[1]), SinCos(value[2], cosOut[2]), SinCos(value[3], cosOut[3])};
#endif
	}
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2>
	SinCos(const Vectorization::Packed<double, 2> value, Vectorization::Packed<double, 2>& cosOut) noexcept
	{
#if USE_SVML
		return _mm_sincos_pd(&cosOut.m_value, value.m_value);
#else
		return {SinCos(value[0], cosOut[0]), SinCos(value[1], cosOut[1])};
#endif
	}
}

#pragma once

#include "Packed.h"

#include <Common/Math/Fract.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Fract(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Fract(value[0]), Fract(value[1]), Fract(value[2]), Fract(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Fract(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Fract(value[0]), Fract(value[1])};
	}
}

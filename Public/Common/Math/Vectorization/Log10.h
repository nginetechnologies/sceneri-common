#pragma once

#include "Packed.h"

#include <Common/Math/Log10.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Log10(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Log10(value[0]), Log10(value[1]), Log10(value[2]), Log10(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Log10(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Log10(value[0]), Log10(value[1])};
	}
}

#pragma once

#include "Packed.h"

#include <Common/Math/Log2.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Log2(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Log2(value[0]), Log2(value[1]), Log2(value[2]), Log2(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Log2(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Log2(value[0]), Log2(value[1])};
	}
}

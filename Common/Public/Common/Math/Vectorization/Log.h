#pragma once

#include "Packed.h"

#include <Common/Math/Log.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Log(const Vectorization::Packed<float, 4> value) noexcept
	{
		return {Log(value[0]), Log(value[1]), Log(value[2]), Log(value[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Log(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Log(value[0]), Log(value[1])};
	}
}

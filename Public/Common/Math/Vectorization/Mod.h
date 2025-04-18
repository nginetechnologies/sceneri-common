#pragma once

#include "Packed.h"

#include <Common/Math/Mod.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4>
	Mod(const Vectorization::Packed<float, 4> a, const Vectorization::Packed<float, 4> b) noexcept
	{
		return {Mod(a[0], b[0]), Mod(a[1], b[1]), Mod(a[2], b[2]), Mod(a[3], b[3])};
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2>
	Mod(const Vectorization::Packed<double, 2> a, const Vectorization::Packed<double, 2> b) noexcept
	{
		return {Mod(a[0], b[0]), Mod(a[1], b[1])};
	}
}

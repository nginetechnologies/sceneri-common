#pragma once

#include "Packed.h"

#include <Common/Math/IsEquivalentTo.h>

namespace ngine::Math
{
	template<typename UnitType, size Count>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS auto IsEquivalentTo(
		const Vectorization::Packed<UnitType, Count> a,
		const Vectorization::Packed<UnitType, Count> b,
		const UnitType epsilon = Math::NumericLimits<UnitType>::Epsilon
	)
	{
		const UnitType epsilonSquared = epsilon * epsilon;
		const Vectorization::Packed<UnitType, Count> diff = a - b;
		return (diff * diff) <= Vectorization::Packed<UnitType, Count>{epsilonSquared};
	}
}

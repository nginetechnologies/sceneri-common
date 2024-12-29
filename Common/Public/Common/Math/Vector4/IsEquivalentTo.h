#pragma once

#include "../Vector4.h"

#include <Common/Math/Vectorization/IsEquivalentTo.h>

namespace ngine::Math
{
	template<typename UnitType>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS auto
	IsEquivalentTo(const TVector4<UnitType> a, const TVector4<UnitType> b, const UnitType epsilon = Math::NumericLimits<UnitType>::Epsilon)
	{
		return IsEquivalentTo<UnitType, 4>(a.m_vectorized, b.m_vectorized, epsilon);
	}
}

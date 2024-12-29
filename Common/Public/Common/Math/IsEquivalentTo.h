#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Math/Abs.h>
#include <Common/Math/NumericLimits.h>

namespace ngine::Math
{
	template<typename Type, typename ReturnType = bool>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS ReturnType
	IsEquivalentTo(const Type a, const Type b, const Type epsilon = Math::NumericLimits<Type>::Epsilon)
	{
		const Type epsilonSquared = epsilon * epsilon;
		const Type diff = a - b;
		return (diff * diff) <= epsilonSquared;
	}
}

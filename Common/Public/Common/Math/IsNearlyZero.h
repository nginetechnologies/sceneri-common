#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Math/Abs.h>
#include <Common/Math/NumericLimits.h>

namespace ngine::Math
{
	template<typename Type, typename ReturnType = bool>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS ReturnType
	IsNearlyZero(const Type value, const Type epsilon = Math::NumericLimits<Type>::Epsilon)
	{
		return Math::Abs(value) <= epsilon;
	}
}

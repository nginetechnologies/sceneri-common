#pragma once

#include <Common/TypeTraits/IsIntegral.h>
#include <Common/TypeTraits/IsFloatingPoint.h>

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsArithmetic = IsIntegral<Type> || IsFloatingPoint<Type>;
}

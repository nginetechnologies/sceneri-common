#pragma once

#include <Common/Platform/UnderlyingType.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsVolatile.h>

namespace ngine::TypeTraits
{
	template<typename TEnum>
	using UnderlyingType = UNDERLYING_TYPE(TEnum);
}

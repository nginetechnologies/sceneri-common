#pragma once

#include "AddReference.h"

namespace ngine::TypeTraits
{
	template<typename Type, typename OtherType = AddLValueReference<const Type>>
	inline static constexpr bool IsCopyConstructible = __is_constructible(Type, OtherType);
}

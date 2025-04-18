#pragma once

#include "AddReference.h"

namespace ngine::TypeTraits
{
	template<typename Type, typename OtherType = const Type&>
	inline static constexpr bool IsCopyAssignable = __is_assignable(Type&, OtherType);
}

#pragma once

namespace ngine::TypeTraits
{
	template<typename Type, typename OtherType = Type>
	inline static constexpr bool IsMoveAssignable = __is_assignable(Type&&, OtherType);
}

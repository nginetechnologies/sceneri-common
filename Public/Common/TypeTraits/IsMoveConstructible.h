#pragma once

namespace ngine::TypeTraits
{
	template<typename Type, typename OtherType = Type>
	inline static constexpr bool IsMoveConstructible = __is_constructible(Type, OtherType);
}

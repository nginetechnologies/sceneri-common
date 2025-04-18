#pragma once

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsEmpty = __is_empty(Type);
}

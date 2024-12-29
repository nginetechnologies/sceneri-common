#pragma once

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsFinal = __is_final(Type);
}

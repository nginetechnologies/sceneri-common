#pragma once

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsDefaultConstructible = __is_constructible(Type);
}

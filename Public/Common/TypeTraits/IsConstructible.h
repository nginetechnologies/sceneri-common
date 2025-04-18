#pragma once

namespace ngine::TypeTraits
{
	template<typename Type, typename... Args>
	inline static constexpr bool IsConstructible = __is_constructible(Type, Args...);
}

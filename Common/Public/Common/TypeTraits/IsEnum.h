#pragma once

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsEnum = __is_enum(Type);
}

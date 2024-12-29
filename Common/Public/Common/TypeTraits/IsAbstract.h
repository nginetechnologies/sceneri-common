#pragma once

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsAbstract = __is_abstract(Type);
}

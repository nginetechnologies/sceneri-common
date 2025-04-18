#pragma once

namespace ngine::TypeTraits
{
	template<typename BaseType, typename DerivedType>
	inline static constexpr bool IsBaseOf = __is_base_of(BaseType, DerivedType);
}

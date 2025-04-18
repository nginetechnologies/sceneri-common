#pragma once

#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/DeclareValue.h>

namespace ngine::TypeTraits
{
	template<typename T1, typename T2, typename = void>
	inline static constexpr bool IsEqualityComparable = false;

	template<typename T1, typename T2>
	inline static constexpr bool IsEqualityComparable<
		T1,
		T2,
		EnableIf<true, decltype(bool(TypeTraits::DeclareValue<T1&>() == TypeTraits::DeclareValue<T2&>()), (void)0)>> = true;
}

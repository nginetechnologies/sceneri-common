#pragma once

#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/DeclareValue.h>

namespace ngine::TypeTraits
{
	template<typename T1, typename T2, typename = void>
	inline static constexpr bool IsPointerComparable = false;

	template<typename T1, typename T2>
	inline static constexpr bool
		IsPointerComparable<T1, T2, EnableIf<true, decltype(bool(DeclareValue<T1*>() == DeclareValue<T2*>()), (void)0)>> = true;
}

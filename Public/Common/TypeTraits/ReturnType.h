#pragma once

#include <Common/TypeTraits/GetFunctionSignature.h>

namespace ngine::TypeTraits
{
	template<typename Type>
	using ReturnType = typename GetFunctionSignature<Type>::ReturnType;
}

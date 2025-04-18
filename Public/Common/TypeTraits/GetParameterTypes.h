#pragma once

#include <Common/TypeTraits/GetFunctionSignature.h>

namespace ngine::TypeTraits
{
	template<typename Type>
	using GetParameterTypes = typename GetFunctionSignature<Type>::ArgumentTypes;
}

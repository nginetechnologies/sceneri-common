#pragma once

#include <Common/TypeTraits/AddReference.h>

namespace ngine::TypeTraits
{
	template<class Type>
	AddRValueReference<Type> DeclareValue() noexcept;
}

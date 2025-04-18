#pragma once

#include "Void.h"

namespace ngine::TypeTraits
{
	template<typename, typename = void>
	inline static constexpr bool IsValidDefinition = false;

	template<typename T>
	inline static constexpr bool IsValidDefinition<T, Void<decltype(sizeof(T))>> = true;
}

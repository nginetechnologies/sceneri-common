#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename Type>
		static auto checkHasFunctionCallOperator(int) -> decltype(&Type::operator(), uint8());
		template<typename Type>
		static uint16 checkHasFunctionCallOperator(...);
		template<typename Type>
		inline static constexpr bool HasFunctionCallOperator = sizeof(checkHasFunctionCallOperator<Type>(0)) == sizeof(uint8);
	}

	template<typename Type>
	inline static constexpr bool HasFunctionCallOperator = Internal::HasFunctionCallOperator<Type>;
}

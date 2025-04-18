#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::TypeTraits
{
	template<typename Type>
	inline static constexpr bool IsStringLiteral = false;

	template<size Count>
	inline static constexpr bool IsStringLiteral<const char (&)[Count]> = true;
}

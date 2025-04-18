#pragma once

#include <Common/Math/Floor.h>

namespace ngine::Math
{
	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr Type Fract(const Type value) noexcept
	{
		return value - Math::Floor(value);
	}
}

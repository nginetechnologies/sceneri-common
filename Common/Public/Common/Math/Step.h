#pragma once

#include <Common/Math/Select.h>

namespace ngine::Math
{
	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr Type Step(const Type edge, const Type x) noexcept
	{
		return Select(x < edge, Type{0}, Type{1});
	}
}

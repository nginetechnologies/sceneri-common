#pragma once

#include "../Vector2.h"
#include "../Hash.h"

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE size Hash(const TVector2<T>& value) noexcept
	{
		return Hash(value.x, value.y);
	}
}

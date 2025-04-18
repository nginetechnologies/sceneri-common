#pragma once

#include <Common/Math/Random.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Random(const TVector3<T> min, const TVector3<T> max) noexcept
	{
		return {Random(min.x, max.x), Random(min.y, max.y), Random(min.z, max.z)};
	}
}

#pragma once

#include <Common/Math/Random.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Random(const TVector4<T> min, const TVector4<T> max) noexcept
	{
		return {Random(min.x, max.x), Random(min.y, max.y), Random(min.z, max.z), Random(min.w, max.w)};
	}
}

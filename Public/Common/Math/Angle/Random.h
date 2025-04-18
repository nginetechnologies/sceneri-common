#pragma once

#include <Common/Math/Random.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] inline TAngle<T> Random(const TAngle<T> min, const TAngle<T> max) noexcept
	{
		return TAngle<T>::FromRadians(Random(min.GetRadians(), max.GetRadians()));
	}
}

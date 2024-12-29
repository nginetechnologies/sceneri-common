#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Power.h>
#include <Common/Math/Vectorization/Ceil.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector2<T> Exponential(const TVector2<T> value) noexcept
	{
		return {Math::Exponential(value.x), Math::Exponential(value.y)};
	}
}

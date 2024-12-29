#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Mod.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector2<T> Mod(const TVector2<T> x, const TVector2<T> y) noexcept
	{
		return {Math::Mod(x.x, y.x), Math::Mod(x.y, y.y)};
	}
}

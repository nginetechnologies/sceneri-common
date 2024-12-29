#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Mod.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector3<T> Mod(const TVector3<T> x, const TVector3<T> y) noexcept
	{
		return {Math::Mod(x.x, y.x), Math::Mod(x.y, y.y), Math::Mod(x.z, y.z)};
	}
}

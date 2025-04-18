#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Mod.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector4<T> Mod(const TVector4<T> x, const TVector4<T> y) noexcept
	{
		return {Math::Mod(x.x, y.x), Math::Mod(x.y, y.y), Math::Mod(x.z, y.z), Math::Mod(x.w, y.w)};
	}
}

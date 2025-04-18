#pragma once

#include <Common/Math/Vector3/Mod.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE WorldCoordinate Mod(const WorldCoordinate x, const WorldCoordinate y) noexcept
	{
		return (WorldCoordinate)Math::Mod((WorldCoordinate::BaseType)x, (WorldCoordinate::BaseType)y);
	}
}

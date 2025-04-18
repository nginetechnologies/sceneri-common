#pragma once

#include <Common/Math/WorldCoordinate.h>
#include <Common/Math/Vector3/Max.h>

namespace ngine::Math
{
	template<>
	[[nodiscard]] FORCE_INLINE constexpr auto Max(const WorldCoordinate a, const WorldCoordinate b) noexcept
	{
		return (WorldCoordinate)Math::Max((WorldCoordinate::BaseType)a, (WorldCoordinate::BaseType)b);
	}
}

#pragma once

#include <Common/Math/WorldCoordinate.h>
#include <Common/Math/Vector3/Min.h>

namespace ngine::Math
{
	template<>
	[[nodiscard]] FORCE_INLINE constexpr auto Min(const WorldCoordinate a, const WorldCoordinate b) noexcept
	{
		return (WorldCoordinate)Math::Min((WorldCoordinate::BaseType)a, (WorldCoordinate::BaseType)b);
	}
}

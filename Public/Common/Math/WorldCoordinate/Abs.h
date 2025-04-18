#pragma once

#include <Common/Math/Vector3/Abs.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE constexpr WorldCoordinate Abs(const WorldCoordinate value) noexcept
	{
		return (WorldCoordinate)Math::Abs((WorldCoordinate::BaseType)value);
	}
}

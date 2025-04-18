#pragma once

#include <Common/Math/Vector3/SignNonZero.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE constexpr WorldCoordinate SignNonZero(const WorldCoordinate value) noexcept
	{
		return (WorldCoordinate)Math::SignNonZero((WorldCoordinate::BaseType)value);
	}
}

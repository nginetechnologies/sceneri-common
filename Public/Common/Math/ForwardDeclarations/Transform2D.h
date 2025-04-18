#pragma once

#include "Vector2.h"
#include "Angle.h"
#include "Rotation2D.h"
#include "WorldCoordinate.h"

namespace ngine::Math
{
	template<typename UnitType>
	struct TTransform2D;

	using Transform2Df = TTransform2D<float>;
	using Coordinate2Df = TVector2<float>;
	using Scale2Df = TVector2<float>;

	using WorldTransform2D = TTransform2D<WorldCoordinateUnitType>;
	using WorldCoordinate2D = TVector2<WorldCoordinateUnitType>;
	using WorldScale2D = TVector2<float>;
}

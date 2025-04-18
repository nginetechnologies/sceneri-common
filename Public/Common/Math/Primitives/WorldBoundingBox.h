#pragma once

#include "ForwardDeclarations/WorldBoundingBox.h"
#include "BoundingBox.h"
#include <Common/Math/WorldCoordinate.h>

#include <Common/Math/WorldCoordinate/Min.h>
#include <Common/Math/WorldCoordinate/Max.h>

namespace ngine::Math
{
	inline WorldBoundingBox operator+(const Math::WorldCoordinate coordinate, const BoundingBox boundingBox)
	{
		return {coordinate + boundingBox.GetMinimum(), coordinate + boundingBox.GetMaximum()};
	}
}

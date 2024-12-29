#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>
#include <Common/Math/ForwardDeclarations/WorldCoordinate.h>

namespace ngine::Math
{
	template<typename VectorType>
	struct TInfinitePlane;
	using InfinitePlanef = TInfinitePlane<Vector3f>;
	using WorldInfinitePlane = TInfinitePlane<WorldCoordinate>;
}

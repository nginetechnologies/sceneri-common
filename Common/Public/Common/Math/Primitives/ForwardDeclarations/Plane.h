#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>
#include <Common/Math/ForwardDeclarations/WorldCoordinate.h>

namespace ngine::Math
{
	template<typename VectorType>
	struct TPlane;
	using Planef = TPlane<Vector3f>;
	using WorldPlane = TPlane<WorldCoordinate>;
}

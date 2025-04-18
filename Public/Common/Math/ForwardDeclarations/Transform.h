#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>

namespace ngine::Math
{
	template<typename CoordinateType, typename RotationUnitType>
	struct TTransform;

	using Transform3Df = TTransform<Vector3f, float>;

	struct WorldTransform;
	struct LocalTransform;
}

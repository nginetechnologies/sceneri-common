#pragma once

#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Transform.h>

namespace ngine::Math
{
	template<typename VectorType, typename CoordinateType, typename RotationUnitType>
	inline TLine<Vector3f> InverseTransform(const TTransform<CoordinateType, RotationUnitType>& transform, const TLine<VectorType> line)
	{
		return Math::Linef{transform.InverseTransformLocation(line.GetStart()), transform.InverseTransformLocation(line.GetEnd())};
	}
}

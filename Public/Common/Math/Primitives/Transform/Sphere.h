#pragma once

#include <Common/Math/Primitives/Sphere.h>
#include <Common/Math/Transform.h>

namespace ngine::Math
{
	template<typename VectorType, typename CoordinateType, typename RotationUnitType>
	inline TSphere<Vector3f> InverseTransform(const TTransform<CoordinateType, RotationUnitType>& transform, const TSphere<VectorType> sphere)
	{
		return Math::TSphere<Vector3f>(
			transform.InverseTransformLocation(sphere.GetPosition()),
			Radiusf::FromMeters(transform.InverseTransformScale(VectorType{sphere.GetRadius().GetMeters()}).x)
		);
	}
}

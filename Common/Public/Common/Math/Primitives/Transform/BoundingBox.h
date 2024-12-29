#pragma once

#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Transform.h>
#include <Common/Math/Vector3/Min.h>
#include <Common/Math/Vector3/Max.h>

namespace ngine::Math
{
	template<typename TransformCoordinateType, typename CoordinateType, typename RotationUnitType>
	FORCE_INLINE TBoundingBox<CoordinateType>
	Transform(const TTransform<CoordinateType, RotationUnitType>& transform, const TBoundingBox<TransformCoordinateType> boundingBox)
	{
		const CoordinateType halfSize = transform.TransformDirection(boundingBox.GetSize() * 0.5f);
		const CoordinateType center = transform.TransformLocation(boundingBox.GetCenter());
		const CoordinateType minimum = center - halfSize;
		const CoordinateType maximum = center + halfSize;

		return {Math::Min(minimum, maximum), Math::Max(minimum, maximum)};
	}

	template<typename TransformCoordinateType, typename CoordinateType, typename RotationUnitType>
	FORCE_INLINE TBoundingBox<Vector3f>
	InverseTransform(const TTransform<CoordinateType, RotationUnitType>& transform, const TBoundingBox<TransformCoordinateType> boundingBox)
	{
		const CoordinateType halfSize = transform.InverseTransformDirection(boundingBox.GetSize() * 0.5f);
		const CoordinateType center = transform.InverseTransformLocation(boundingBox.GetCenter());
		const CoordinateType minimum = center - halfSize;
		const CoordinateType maximum = center + halfSize;
		return {Math::Min(minimum, maximum), Math::Max(minimum, maximum)};
	}
}

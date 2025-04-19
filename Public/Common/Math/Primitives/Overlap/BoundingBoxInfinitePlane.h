#pragma once

#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Primitives/InfinitePlane.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] bool Overlaps(const TBoundingBox<VectorType> boundingBox, const TInfinitePlane<VectorType> plane)
	{
		using UnitType = typename VectorType::UnitType;

		const VectorType center = boundingBox.GetCenter();
		const VectorType extents = boundingBox.GetMaximum() - center;

		const VectorType r3 = extents * Abs(plane.GetNormal());
		const UnitType r = r3.GetSum();
		const UnitType s = plane.GetNormal().Dot(center) - plane.GetDistance();

		return Abs(s) <= r;
	}

	template<typename VectorType>
	[[nodiscard]] inline bool Overlaps(const TInfinitePlane<VectorType> plane, const TBoundingBox<VectorType> boundingBox)
	{
		return Overlaps<VectorType>(boundingBox, plane);
	}

	inline bool Overlaps(const Math::WorldInfinitePlane& plane, const Math::WorldBoundingBox& boundingBox)
	{
		return Overlaps<Math::WorldCoordinate>(plane, boundingBox);
	}
	inline bool Overlaps(const Math::WorldBoundingBox& boundingBox, const Math::WorldInfinitePlane& plane)
	{
		return Overlaps<Math::WorldCoordinate>(plane, boundingBox);
	}
	inline bool Overlaps(const Math::InfinitePlanef& plane, const Math::BoundingBox& boundingBox)
	{
		return Overlaps<Math::Vector3f>(plane, boundingBox);
	}
	inline bool Overlaps(const Math::BoundingBox& boundingBox, const Math::InfinitePlanef& plane)
	{
		return Overlaps<Math::Vector3f>(plane, boundingBox);
	}
}

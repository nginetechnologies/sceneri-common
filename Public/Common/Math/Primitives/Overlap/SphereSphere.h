#pragma once

#include <Common/Math/Primitives/Overlap.h>
#include <Common/Math/Primitives/Sphere.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] FORCE_INLINE bool Overlaps(const TSphere<VectorType>& left, const Math::TSphere<VectorType>& right)
	{
		const float squaredDistance = (left.GetPosition() - right.GetPosition()).GetLengthSquared();
		const float radius = left.GetRadius().GetMeters() + right.GetRadius().GetMeters();
		const float squaredRadius = radius * radius;
		return squaredRadius > squaredDistance;
	}

	FORCE_INLINE bool Overlaps(const Math::WorldSphere& left, const Math::WorldSphere& right)
	{
		return Overlaps<Math::WorldCoordinate>(left, right);
	}
}

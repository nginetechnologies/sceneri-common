#pragma once

#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/Intersect/Result.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] TIntersectionResult<VectorType> Intersects(const TLine<VectorType>& line1, const TLine<VectorType>& line2)
	{
		const VectorType distance1 = line1.GetDistance();
		const VectorType distance2 = line2.GetDistance();
		const VectorType distanceBetweenStarts = line2.GetStart() - line1.GetStart();

		const VectorType distanceCross = distance1.Cross(distance2);
		if (distanceBetweenStarts.Dot(distanceCross) != 0.f)
		{
			return TIntersectionResult<VectorType>{IntersectionType::NoIntersection};
		}

		const typename VectorType::UnitType s = distanceBetweenStarts.Cross(distance2).Dot(distanceCross) / distanceCross.GetLengthSquared();
		if ((s >= 0.f) & (s <= 1.f))
		{
			return TIntersectionResult<VectorType>{IntersectionType::Intersection, line1.GetStart() + distance1 * VectorType(s)};
		}

		return TIntersectionResult<VectorType>{IntersectionType::NoIntersection};
	}
}

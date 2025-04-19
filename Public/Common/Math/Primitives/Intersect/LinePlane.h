#pragma once

#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/Plane.h>
#include <Common/Math/Primitives/Intersect/Result.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] TIntersectionResult<VectorType> Intersects(const TLine<VectorType> line, const TPlane<VectorType> plane)
	{
		const VectorType lineDistance = line.GetDistance();
		const VectorType startToPlaneDistance = line.GetStart() - plane.GetPosition();

		using UnitType = typename VectorType::UnitType;

		const UnitType D = plane.GetNormal().Dot(lineDistance);
		const UnitType N = -plane.GetNormal().Dot(startToPlaneDistance);

		if (Math::Abs(D) < UnitType(0.00001))
		{
			return TIntersectionResult<VectorType>{N == 0 ? IntersectionType::Inside : IntersectionType::NoIntersection};
		}

		const UnitType ND = N / D;
		if ((ND < 0) | (ND > 1))
		{
			return TIntersectionResult<VectorType>{IntersectionType::NoIntersection};
		}

		return TIntersectionResult<VectorType>{IntersectionType::Intersection, line.GetStart() + lineDistance * ND, plane.GetNormal()};
	}

	template<typename VectorType>
	[[nodiscard]] TIntersectionResult<VectorType> Intersects(const TPlane<VectorType> plane, const TLine<VectorType> line)
	{
		return Intersects(line, plane);
	}
}

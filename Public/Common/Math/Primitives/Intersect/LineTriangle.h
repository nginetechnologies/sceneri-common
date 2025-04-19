#pragma once

#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/Triangle.h>
#include <Common/Math/Primitives/Intersect.h>
#include <Common/Math/Primitives/Intersect/Result.h>
#include <Common/Math/Primitives/Intersect/ForwardDeclarations/WorldResult.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] TIntersectionResult<VectorType> Intersects(const TLine<VectorType> line, const TTriangle<VectorType> triangle)
	{
		using UnitType = typename VectorType::UnitType;
		const VectorType lineDistance = line.GetDistance();

		const VectorType edgeA = triangle[1] - triangle[0];
		const VectorType p = lineDistance.Cross(edgeA);

		const VectorType edgeB = triangle[2] - triangle[0];
		const VectorType distanceToStart = line.GetStart() - triangle[0];
		const VectorType q = distanceToStart.Cross(edgeB);
		const Math::TVector2<UnitType> uv = {distanceToStart.Dot(p), lineDistance.Dot(q)};

		const UnitType dot = edgeB.Dot(p);
		const UnitType uvLessThanDot = dot - uv.GetSum();
		const UnitType UUVLessThanDot = Math::Select(uvLessThanDot >= 0, dot - uv.x, uvLessThanDot);
		const UnitType uvGreaterEqualThanZero = Math::Select(uv.y >= 0, uv.x, uv.y);
		const UnitType BothGood = Math::Select(uvGreaterEqualThanZero >= 0, UUVLessThanDot, uvGreaterEqualThanZero);

		constexpr UnitType Epsilon = UnitType(0.0000001);
		const UnitType dotGreaterThanEpsilon = dot - Epsilon;
		if (Math::Select(dotGreaterThanEpsilon >= 0, BothGood, dotGreaterThanEpsilon) < 0)
		{
			return TIntersectionResult<VectorType>{IntersectionType::NoIntersection};
		}

		const UnitType t = edgeA.Dot(q) / dot;

		const VectorType result = line.GetStart() + (lineDistance * t);

		const UnitType afterStart = (result - line.GetStart()).Dot(lineDistance);
		const UnitType beforeEnd = -(result - line.GetEnd()).Dot(lineDistance);
		const UnitType isWithin = Math::Select(afterStart >= 0, beforeEnd, afterStart);

		return TIntersectionResult<VectorType>{
			Math::Select(isWithin >= 0, IntersectionType::Intersection, IntersectionType::NoIntersection),
			result,
			edgeB.Cross(edgeA).GetNormalized()
		};
	}

	template<typename VectorType>
	[[nodiscard]] inline TIntersectionResult<VectorType> Intersects(const TTriangle<VectorType> triangle, const TLine<VectorType> line)
	{
		return Intersects(line, triangle);
	}

	inline Math::WorldIntersectionResult Intersects(const Math::WorldLine& line, const Math::WorldTriangle& triangle)
	{
		return Intersects<Math::WorldCoordinate>(line, triangle);
	}
	inline Math::WorldIntersectionResult Intersects(const Math::WorldTriangle& triangle, const Math::WorldLine& line)
	{
		return Intersects<Math::WorldCoordinate>(line, triangle);
	}

	inline Math::TIntersectionResult<Vector3f> Intersects(const Math::Linef& line, const Math::Trianglef& triangle)
	{
		return Intersects<Math::Vector3f>(line, triangle);
	}
	inline Math::TIntersectionResult<Vector3f> Intersects(const Math::Trianglef& triangle, const Math::Linef& line)
	{
		return Intersects<Math::Vector3f>(line, triangle);
	}
}

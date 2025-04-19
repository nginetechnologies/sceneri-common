#pragma once

#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Primitives/Triangle.h>
#include <Common/Math/Primitives/InfinitePlane.h>
#include <Common/Math/Primitives/Overlap/BoundingBoxInfinitePlane.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] bool Overlaps(const TBoundingBox<VectorType> boundingBox, const TTriangle<VectorType> triangle)
	{
		using UnitType = typename VectorType::UnitType;

		const VectorType center = boundingBox.GetCenter();
		const VectorType extents = boundingBox.GetMaximum() - center;

		const VectorType v0 = triangle[0] - center;
		const VectorType v1 = triangle[1] - center;
		const VectorType v2 = triangle[2] - center;

		// Compute edge vectors for triangle
		const VectorType f0 = v1 - v0;
		const VectorType f1 = v2 - v1;
		const VectorType f2 = v0 - v2;

		// Test axes a00..a22 (category 3)
		const VectorType a00 = {0, -f0.z, f0.y};
		const VectorType a01 = {0, -f1.z, f1.y};
		const VectorType a02 = {0, -f2.z, f2.y};
		const VectorType a10 = {f0.z, 0, -f0.x};
		const VectorType a11 = {f1.z, 0, -f1.x};
		const VectorType a12 = {f2.z, 0, -f2.x};
		const VectorType a20 = {-f0.y, f0.x, 0};
		const VectorType a21 = {-f1.y, f1.x, 0};
		const VectorType a22 = {-f2.y, f2.x, 0};

		// Test axis a00
		UnitType p0 = v0.Dot(a00);
		UnitType p1 = v1.Dot(a00);
		UnitType p2 = v2.Dot(a00);
		UnitType r = extents.y * Abs(f0.z) + extents.z * Abs(f0.y);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a01
		p0 = v0.Dot(a01);
		p1 = v1.Dot(a01);
		p2 = v2.Dot(a01);
		r = extents.y * Abs(f1.z) + extents.z * Abs(f1.y);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a02
		p0 = v0.Dot(a02);
		p1 = v1.Dot(a02);
		p2 = v2.Dot(a02);
		r = extents.y * Abs(f2.z) + extents.z * Abs(f2.y);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a10
		p0 = v0.Dot(a10);
		p1 = v1.Dot(a10);
		p2 = v2.Dot(a10);
		r = extents.x * Abs(f0.z) + extents.z * Abs(f0.x);
		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a11
		p0 = v0.Dot(a11);
		p1 = v1.Dot(a11);
		p2 = v2.Dot(a11);
		r = extents.x * Abs(f1.z) + extents.z * Abs(f1.x);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a12
		p0 = v0.Dot(a12);
		p1 = v1.Dot(a12);
		p2 = v2.Dot(a12);
		r = extents.x * Abs(f2.z) + extents.z * Abs(f2.x);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a20
		p0 = v0.Dot(a20);
		p1 = v1.Dot(a20);
		p2 = v2.Dot(a20);
		r = extents.x * Abs(f0.y) + extents.y * Abs(f0.x);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a21
		p0 = v0.Dot(a21);
		p1 = v1.Dot(a21);
		p2 = v2.Dot(a21);
		r = extents.x * Abs(f1.y) + extents.y * Abs(f1.x);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test axis a22
		p0 = v0.Dot(a22);
		p1 = v1.Dot(a22);
		p2 = v2.Dot(a22);
		r = extents.x * Abs(f2.y) + extents.y * Abs(f2.x);

		if (Max(-Max(p0, p1, p2), Min(p0, p1, p2)) > r)
		{

			return false; // Axis is a separating axis
		}

		// Test the three axes corresponding to the face normals of AABB b (category 1).
		// Exit if...
		// ... [-extents.x, extents.x] and [min(v0.x,v1.x,v2.x), max(v0.x,v1.x,v2.x)] do not overlap
		if (Max(v0.x, v1.x, v2.x) < -extents.x || Min(v0.x, v1.x, v2.x) > extents.x)
		{

			return false;
		}
		// ... [-extents.y, extents.y] and [min(v0.y,v1.y,v2.y), max(v0.y,v1.y,v2.y)] do not overlap
		if (Max(v0.y, v1.y, v2.y) < -extents.y || Min(v0.y, v1.y, v2.y) > extents.y)
		{

			return false;
		}
		// ... [-extents.z, extents.z] and [min(v0.z,v1.z,v2.z), max(v0.z,v1.z,v2.z)] do not overlap
		if (Max(v0.z, v1.z, v2.z) < -extents.z || Min(v0.z, v1.z, v2.z) > extents.z)
		{

			return false;
		}

		// Test separating axis corresponding to triangle face normal (category 2)
		// Face Normal is -ve as Triangle is clockwise winding (and XNA uses -z for into screen)
		Math::TInfinitePlane<VectorType> plane(f1.Cross(f0).GetNormalized(), triangle[0]);
		return Math::Overlaps(plane, boundingBox);
	}

	template<typename VectorType>
	[[nodiscard]] inline bool Overlaps(const TTriangle<VectorType> triangle, const TBoundingBox<VectorType> boundingBox)
	{
		return Overlaps(boundingBox, triangle);
	}

	inline bool Overlaps(const Math::WorldTriangle& triangle, const Math::WorldBoundingBox& boundingBox)
	{
		return Overlaps<Math::WorldCoordinate>(triangle, boundingBox);
	}
	inline bool Overlaps(const Math::WorldBoundingBox& boundingBox, const Math::WorldTriangle& triangle)
	{
		return Overlaps<Math::WorldCoordinate>(triangle, boundingBox);
	}
}

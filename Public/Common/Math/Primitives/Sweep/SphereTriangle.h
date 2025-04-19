#pragma once

#include <Common/Math/Primitives/Sphere.h>
#include <Common/Math/Primitives/Triangle.h>
#include <Common/Math/Primitives/Sweep/Result.h>
#include <Common/Math/Sqrt.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] TSweepResult<VectorType>
	Sweep(const TSphere<VectorType> sphere, const Math::Vector3f sweepDistance, const Math::TTriangle<VectorType> triangle)
	{
		using UnitType = typename VectorType::UnitType;

		const VectorType closestPoint = triangle.GetClosestPoint(sphere.GetPosition());
		const UnitType distanceSquared = (closestPoint - sphere.GetPosition()).GetLengthSquared();
		if (distanceSquared <= sphere.GetRadiusSquared())
		{
			return TSweepResult<VectorType>{
				distanceSquared < sphere.GetRadiusSquared() ? SweepIntersectionType::InitialOverlap : SweepIntersectionType::Intersection,
				0.f,
				closestPoint
			};
		}

		const UnitType sqrLenV = sweepDistance.GetLength();

		// Compute the triangle edge directions E[], the vector U normal
		// to the plane of the triangle,  and compute the normals to the
		// edges in the plane of the triangle.  TODO: For a nondeforming
		// triangle (or mesh of triangles), these quantities can all be
		// precomputed to reduce the computational cost of the query.  Add
		// another operator()-query that accepts the precomputed values.
		// TODO: When the triangle is deformable, these quantities must be
		// computed, either by the caller or here.  Optimize the code to
		// compute the quantities on-demand (i.e. only when they are
		// needed, but cache them for later use).
		VectorType E[3] = {triangle[1] - triangle[0], triangle[2] - triangle[1], triangle[0] - triangle[2]};
		UnitType sqrLenE[3] = {E[0].Dot(E[0]), E[1].Dot(E[1]), E[2].Dot(E[2])};
		VectorType U = E[0].Cross(E[1]);
		VectorType ExU[3] = {E[0].Cross(U), E[1].Cross(U), E[2].Cross(U)};

		// Compute the vectors from the triangle vertices to the sphere
		// center.
		VectorType Delta[3] = {sphere.GetPosition() - triangle[0], sphere.GetPosition() - triangle[1], sphere.GetPosition() - triangle[2]};

		// Determine where the sphere center is located relative to the
		// planes of the triangle offset faces of the sphere-swept volume.
		UnitType dotUDelta0 = U.Dot(Delta[0]);
		if (dotUDelta0 >= sphere.GetRadius())
		{
			// The sphere is on the positive side of Dot(U,X-C) = r.  If
			// the sphere will contact the sphere-swept volume at a
			// triangular face, it can do so only on the face of the
			// aforementioned plane.
			UnitType dotUV = U.Dot(sweepDistance);
			if (dotUV >= (UnitType)0)
			{
				// The sphere is moving away from, or parallel to, the
				// plane of the triangle.
				return TSweepResult<VectorType>{SweepIntersectionType::None};
			}

			UnitType tbar = (sphere.GetRadius() - dotUDelta0) / dotUV;
			bool foundContact = true;
			for (uint8 i = 0; i < 3; ++i)
			{
				UnitType phi = ExU[i].Dot(Delta[i]);
				UnitType psi = ExU[i].Dot(sweepDistance);
				if (phi + psi * tbar > (UnitType)0)
				{
					foundContact = false;
					break;
				}
			}

			if (foundContact)
			{
				return TSweepResult<VectorType>{SweepIntersectionType::Intersection, tbar, sphere.GetPosition() + tbar * sweepDistance};
			}
		}
		else if (dotUDelta0 <= -sphere.GetRadius())
		{
			// The sphere is on the positive side of Dot(-U,X-C) = r.  If
			// the sphere will contact the sphere-swept volume at a
			// triangular face, it can do so only on the face of the
			// aforementioned plane.
			UnitType dotUV = U.Dot(sweepDistance);
			if (dotUV <= (UnitType)0)
			{
				// The sphere is moving away from, or parallel to, the
				// plane of the triangle.
				return TSweepResult<VectorType>{SweepIntersectionType::None};
			}

			UnitType tbar = (-sphere.GetRadius() - dotUDelta0) / dotUV;
			bool foundContact = true;
			for (uint8 i = 0; i < 3; ++i)
			{
				UnitType phi = ExU[i].Dot(Delta[i]);
				UnitType psi = ExU[i].Dot(sweepDistance);
				if (phi + psi * tbar > (UnitType)0)
				{
					foundContact = false;
					break;
				}
			}
			if (foundContact)
			{
				return TSweepResult<VectorType>{SweepIntersectionType::Intersection, tbar, sphere.GetPosition() + tbar * sweepDistance};
			}
		}
		// else: The ray-sphere-swept-volume contact point (if any) cannot
		// be on a triangular face of the sphere-swept-volume.

		// The sphere is moving towards the slab between the two planes
		// of the sphere-swept volume triangular faces.  Determine whether
		// the ray intersects the half cylinders or sphere wedges of the
		// sphere-swept volume.

		// Test for contact with half cylinders of the sphere-swept
		// volume.  First, precompute some dot products required in the
		// computations.  TODO: Optimize the code to compute the quantities
		// on-demand (i.e. only when they are needed, but cache them for
		// later use).
		UnitType del[3], delp[3], nu[3];
		for (uint8 im1 = 2, i = 0; i < 3; im1 = i++)
		{
			del[i] = E[i].Dot(Delta[i]);
			delp[im1] = E[im1].Dot(Delta[i]);
			nu[i] = E[i].Dot(sweepDistance);
		}

		for (uint8 i = 2, ip1 = 0; ip1 < 3; i = ip1++)
		{
			VectorType hatV = sweepDistance - E[i] * nu[i] / sqrLenE[i];
			UnitType sqrLenHatV = hatV.Dot(hatV);
			if (sqrLenHatV > (UnitType)0)
			{
				VectorType hatDelta = Delta[i] - E[i] * del[i] / sqrLenE[i];
				UnitType alpha = -hatV.Dot(hatDelta);
				if (alpha >= (UnitType)0)
				{
					UnitType sqrLenHatDelta = hatDelta.Dot(hatDelta);
					UnitType beta = alpha * alpha - sqrLenHatV * (sqrLenHatDelta - sphere.GetRadiusSquared());
					if (beta >= (UnitType)0)
					{
						UnitType tbar = (alpha - Math::Sqrt(beta)) / sqrLenHatV;

						UnitType mu = ExU[i].Dot(Delta[i]);
						UnitType omega = ExU[i].Dot(hatV);
						if (mu + omega * tbar >= (UnitType)0)
						{
							if (del[i] + nu[i] * tbar >= (UnitType)0)
							{
								if (delp[i] + nu[i] * tbar <= (UnitType)0)
								{
									return TSweepResult<VectorType>{SweepIntersectionType::Intersection, tbar, sphere.GetPosition() + tbar * sweepDistance};
								}
							}
						}
					}
				}
			}
		}

		// Test for contact with sphere wedges of the sphere-swept
		// volume.  We know that |V|^2 > 0 because of a previous
		// early-exit test.
		for (int im1 = 2, i = 0; i < 3; im1 = i++)
		{
			UnitType alpha = -sweepDistance.Dot(Delta[i]);
			if (alpha >= (UnitType)0)
			{
				UnitType sqrLenDelta = Delta[i].Dot(Delta[i]);
				UnitType beta = alpha * alpha - sqrLenV * (sqrLenDelta - sphere.GetRadiusSquared());
				if (beta >= (UnitType)0)
				{
					UnitType tbar = (alpha - Math::Sqrt(beta)) / sqrLenV;
					if (delp[im1] + nu[im1] * tbar >= (UnitType)0)
					{
						if (del[i] + nu[i] * tbar <= (UnitType)0)
						{
							// The constraints are satisfied, so tbar
							// is the first time of contact.
							return TSweepResult<VectorType>{SweepIntersectionType::Intersection, tbar, sphere.GetPosition() + tbar * sweepDistance};
						}
					}
				}
			}
		}

		// The ray and sphere-swept volume do not intersect, so the sphere
		// and triangle do not come into contact.  The 'result' is already
		// set to the correct state for this case.
		return TSweepResult<VectorType>{SweepIntersectionType::None};
	}

	template<typename VectorType>
	[[nodiscard]] TSweepResult<VectorType>
	Sweep(const Math::TTriangle<VectorType> triangle, const TSphere<VectorType> sphere, const Math::Vector3f sweepDistance)
	{
		return Sweep(sphere, sweepDistance, triangle);
	}
}

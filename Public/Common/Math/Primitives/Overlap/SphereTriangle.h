#pragma once

#include <Common/Math/Primitives/Overlap.h>
#include <Common/Math/Primitives/Overlap/SphereSphere.h>
#include <Common/Math/Primitives/Triangle.h>
#include <Common/Math/Primitives/Sphere.h>
#include <Common/Math/Vector3/Select.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] bool Overlaps(const TSphere<VectorType>& sphere, const Math::TTriangle<VectorType>& triangle)
	{
		using UnitType = typename VectorType::UnitType;

		const VectorType triangleCenter = triangle.GetCenter();
		const UnitType radiusSquared = Math::Max(
			(triangle[0] - triangleCenter).GetLengthSquared(),
			(triangle[1] - triangleCenter).GetLengthSquared(),
			(triangle[2] - triangleCenter).GetLengthSquared()
		);
		if (!Overlaps(sphere, TSphere<VectorType>(triangleCenter, Radius<UnitType>::FromMeters(Sqrt(radiusSquared)))))
		{
			return false;
		}

		const VectorType sphereCenter = sphere.GetPosition();
		const VectorType A(triangle[0] - sphereCenter);
		const VectorType B(triangle[1] - sphereCenter);
		const VectorType C(triangle[2] - sphereCenter);
		const UnitType sphereRadiusSquared = sphere.GetRadiusSquared();
		const VectorType AB = B - A;
		const VectorType V = AB.Cross(C - A);
		const UnitType d = A.Dot(V);
		const UnitType d2 = d * d;
		const UnitType e = V.GetLengthSquared();

		if (d2 > sphereRadiusSquared * e)
		{
			return false;
		}

		const VectorType pointDistancesToCenterSquared = {A.GetLengthSquared(), B.GetLengthSquared(), C.GetLengthSquared()};

		const VectorType distanceDots = {A.Dot(B), B.Dot(C), A.Dot(C)};

		{
			const VectorType largerThanSquaredRadius =
				VectorType{pointDistancesToCenterSquared.x, pointDistancesToCenterSquared.y, pointDistancesToCenterSquared.z} >
				VectorType{sphereRadiusSquared};
			const VectorType largerThanDistanceToCenter = VectorType{distanceDots.x, distanceDots.x, distanceDots.z} >
			                                              pointDistancesToCenterSquared;
			const VectorType largerThanDistanceToCenter2 = VectorType{distanceDots.z, distanceDots.y, distanceDots.y} >
			                                               pointDistancesToCenterSquared;

			const typename VectorType::BoolType failedAny = largerThanSquaredRadius & largerThanDistanceToCenter & largerThanDistanceToCenter2;
			if (failedAny.AreAnySet())
			{
				return false;
			}
		}

		{
			const VectorType BC = C - B;
			const VectorType CA = A - C;

			const VectorType trianglePointDistancesSquared = {AB.GetLengthSquared(), BC.GetLengthSquared(), CA.GetLengthSquared()};
			const VectorType trianglePointDistancesSquaredSquared = trianglePointDistancesSquared.GetSquared();
			const VectorType dv2 = distanceDots - pointDistancesToCenterSquared;

			const VectorType Q1 = (A * trianglePointDistancesSquared.x) - (AB * dv2.x);
			const VectorType QC = (C * trianglePointDistancesSquared.x) - Q1;
			const VectorType Q2 = (B * trianglePointDistancesSquared.y) - (BC * dv2.y);
			const VectorType QA = (A * trianglePointDistancesSquared.y) - Q2;
			const VectorType Q3 = (C * trianglePointDistancesSquared.z) - (CA * dv2.z);
			const VectorType QB = (B * trianglePointDistancesSquared.z) - Q3;

			{
				const VectorType QLengthsSquared = {Q1.GetLengthSquared(), Q2.GetLengthSquared(), Q3.GetLengthSquared()};
				const VectorType sphereRadiusSquaredTest = sphereRadiusSquared * trianglePointDistancesSquaredSquared;
				const VectorType QLengthsLargerThanSphereRadiusTest = QLengthsSquared > sphereRadiusSquaredTest;

				const VectorType QDots = {Q1.Dot(QC), Q2.Dot(QA), Q3.Dot(QB)};
				const VectorType QDotsLargerThanZero = QDots >= VectorType(Zero);
				const typename VectorType::BoolType FailedIntersectionTest = QLengthsLargerThanSphereRadiusTest & QDotsLargerThanZero;

				if (FailedIntersectionTest.AreAnySet())
				{
					return false;
				}
			}
		}

		return true;
	}

	template<typename VectorType>
	[[nodiscard]] bool Overlaps(const Math::TTriangle<VectorType>& triangle, const TSphere<VectorType>& sphere)
	{
		return Overlaps(sphere, triangle);
	}

	inline bool Overlaps(const Math::WorldSphere& sphere, const Math::WorldTriangle& triangle)
	{
		return Overlaps<Math::WorldCoordinate>(sphere, triangle);
	}
	inline bool Overlaps(const Math::WorldTriangle& triangle, const Math::WorldSphere& sphere)
	{
		return Overlaps<Math::WorldCoordinate>(sphere, triangle);
	}
}

#pragma once

#include <Common/Math/Primitives/Overlap.h>
#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Primitives/Sphere.h>
#include <Common/Math/Vector3/Select.h>

namespace ngine::Math
{
	template<typename VectorType, typename BoundingBoxVectorType>
	[[nodiscard]] bool Overlaps(const TSphere<VectorType>& sphere, const Math::TBoundingBox<BoundingBoxVectorType>& boundingBox)
	{
		const VectorType boundingMinimumMinusSphereCenter = boundingBox.GetMinimum() - sphere.GetPosition();
		const VectorType sphereCenterMinusBoundingMinimum = sphere.GetPosition() - boundingBox.GetMinimum();
		const VectorType sphereCenterMinusBoundingMaximum = sphere.GetPosition() - boundingBox.GetMaximum();

		const VectorType relativeLocation = Select(
			boundingMinimumMinusSphereCenter >= (VectorType)Math::Zero,
			sphereCenterMinusBoundingMinimum,
			Select(sphereCenterMinusBoundingMaximum >= (VectorType)Math::Zero, sphereCenterMinusBoundingMaximum, (VectorType)Math::Zero)
		);

		return relativeLocation.GetLengthSquared() < sphere.GetRadiusSquared();
	}

	template<typename VectorType, typename BoundingBoxVectorType>
	[[nodiscard]] bool Overlaps(const Math::TBoundingBox<BoundingBoxVectorType>& boundingBox, const TSphere<VectorType>& sphere)
	{
		return Overlaps(sphere, boundingBox);
	}

	inline bool Overlaps(const Math::WorldSphere& sphere, const Math::WorldBoundingBox& boundingBox)
	{
		return Overlaps<Math::WorldSphere::VectorType, Math::WorldBoundingBox::VectorType>(sphere, boundingBox);
	}
	inline bool Overlaps(const Math::WorldBoundingBox& boundingBox, const Math::WorldSphere& sphere)
	{
		return Overlaps<Math::WorldSphere::VectorType, Math::WorldBoundingBox::VectorType>(sphere, boundingBox);
	}
}

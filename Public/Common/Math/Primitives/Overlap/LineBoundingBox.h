#pragma once

#include <Common/Math/Primitives/Overlap.h>
#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Primitives/WorldBoundingBox.h>
#include <Common/Math/Matrix3x3.h>
#include <Common/Math/Vector3/Abs.h>
#include <Common/Math/WorldCoordinate/Abs.h>

namespace ngine::Math
{
	template<typename VectorType, typename BoundingBoxVectorType>
	[[nodiscard]] bool Overlaps(const TLine<VectorType>& line, const Math::TBoundingBox<BoundingBoxVectorType>& boundingBox)
	{
		const VectorType boundingBoxHalfSize = boundingBox.GetSize() * 0.5f;
		const VectorType centerDistance = line.GetCenter() - boundingBox.GetCenter();
		const VectorType halfLineDistance = line.GetDistance() * 0.5f;

		const VectorType halfDistance = boundingBoxHalfSize + Math::Abs(halfLineDistance);
		if ((Math::Abs(centerDistance) > halfDistance).AreAnySet())
		{
			return false;
		}

		const VectorType lhs = Math::Abs(centerDistance * VectorType(halfLineDistance.z, halfLineDistance.x, halfLineDistance.y)) -
		                       (VectorType(centerDistance.z, centerDistance.x, centerDistance.y) * halfDistance);
		const VectorType a = Math::Abs(
			VectorType{boundingBoxHalfSize.x, boundingBoxHalfSize.x, boundingBoxHalfSize.y} *
			VectorType{halfLineDistance.z, halfLineDistance.y, halfLineDistance.z}
		);
		const VectorType b = Math::Abs(
			VectorType{boundingBoxHalfSize.z, boundingBoxHalfSize.y, boundingBoxHalfSize.z} *
			VectorType{halfLineDistance.x, halfLineDistance.x, halfLineDistance.y}
		);
		return (lhs > (a + b)).AreNoneSet();
	}

	template<typename VectorType, typename BoundingBoxVectorType>
	[[nodiscard]] bool Overlaps(const Math::TBoundingBox<BoundingBoxVectorType>& boundingBox, const TLine<VectorType>& line)
	{
		return Overlaps(line, boundingBox);
	}

	inline bool Overlaps(const Math::WorldLine& line, const Math::WorldBoundingBox& boundingBox)
	{
		return Overlaps<Math::WorldLine::VectorType, Math::WorldBoundingBox::VectorType>(line, boundingBox);
	}
	inline bool Overlaps(const Math::WorldBoundingBox& boundingBox, const Math::WorldLine& line)
	{
		return Overlaps<Math::WorldLine::VectorType, Math::WorldBoundingBox::VectorType>(line, boundingBox);
	}

	template<typename VectorType, typename BoundingBoxVectorType>
	[[nodiscard]] bool
	Overlaps(const TLine<VectorType>& line, const Math::TBoundingBox<BoundingBoxVectorType>& boundingBox, Math::Matrix3x3f rotation)
	{
		const VectorType halfSize = rotation.InverseTransformDirection(boundingBox.GetSize() * 0.5f);
		const VectorType centerDistance = rotation.InverseTransformDirection(line.GetCenter() - boundingBox.GetCenter());
		const VectorType halfLineDistance = rotation.InverseTransformDirection(line.GetDistance() * 0.5f);

		if ((Math::Abs(centerDistance) > (halfSize + Math::Abs(halfLineDistance))).AreAllSet())
		{
			return false;
		}

		const VectorType absCross = Math::Abs(halfLineDistance.Cross(centerDistance));

		const VectorType a =
			Math::Abs(VectorType{halfSize.y, halfSize.x, halfSize.x} * VectorType{halfLineDistance.z, halfLineDistance.z, halfLineDistance.y});
		const VectorType b =
			Math::Abs(VectorType{halfSize.z, halfSize.z, halfSize.y} * VectorType{halfLineDistance.y, halfLineDistance.x, halfLineDistance.x});
		return (absCross > a + b).AreNoneSet();
	}

	template<typename VectorType, typename BoundingBoxVectorType>
	[[nodiscard]] bool
	Overlaps(const Math::TBoundingBox<BoundingBoxVectorType>& boundingBox, Math::Matrix3x3f rotation, const TLine<VectorType>& line)
	{
		return Overlaps(line, boundingBox, rotation);
	}
}

#pragma once

#include <Common/Math/Primitives/Overlap.h>
#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Primitives/ForwardDeclarations/WorldBoundingBox.h>

namespace ngine::Math
{
	template<typename BoundingBoxVectorType>
	[[nodiscard]] bool Overlaps(const Math::TBoundingBox<BoundingBoxVectorType>& left, const Math::TBoundingBox<BoundingBoxVectorType>& right)
	{
		const BoundingBoxVectorType leftMinLess = left.GetMinimum() >= right.GetMaximum();
		const BoundingBoxVectorType leftMaxGreater = left.GetMaximum() <= right.GetMinimum();
		return (leftMinLess | leftMaxGreater).AreNoneSet();
	}

	inline bool Overlaps(const Math::WorldBoundingBox& left, const Math::WorldBoundingBox& right)
	{
		return Overlaps<Math::WorldBoundingBox::VectorType>(left, right);
	}
}

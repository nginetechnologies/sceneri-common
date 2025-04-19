#pragma once

#include <Common/Math/Primitives/Intersect/Result.h>

namespace ngine::Math
{
	template<typename VectorType, typename LeftType, typename RightType>
	[[nodiscard]] TIntersectionResult<VectorType> Intersects(const LeftType&, const RightType&) = delete;
}

#pragma once

#include <Common/Math/Primitives/Intersect/Result.h>

namespace ngine::Math
{
	template<typename VectorType, typename LeftType, typename SweptType>
	[[nodiscard]] TIntersectionResult<VectorType>
	Intersects(const LeftType&, const SweptType&, [[maybe_unused]] const Vector3f sweepDirection) = delete;
}

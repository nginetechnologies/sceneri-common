#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	struct TIntersectionResult;

	using IntersectionResultf = TIntersectionResult<Vector3f>;
}

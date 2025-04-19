#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	struct TSweepResult;

	using SweepResultf = TSweepResult<Vector3f>;
}

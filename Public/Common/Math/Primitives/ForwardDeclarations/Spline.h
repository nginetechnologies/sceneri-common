#pragma once

#include <Common/Math/ForwardDeclarations/Vector2.h>
#include <Common/Math/ForwardDeclarations/Vector3.h>

namespace ngine::Math
{
	template<typename CoordinateType_>
	struct Spline;

	using Splinef = Spline<Vector3f>;
	using Spline2f = Spline<Vector2f>;
}

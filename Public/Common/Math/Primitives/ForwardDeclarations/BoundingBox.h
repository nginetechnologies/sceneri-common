#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vector2.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	struct TBoundingBox;
	using BoundingBox = TBoundingBox<TVector3<float>>;
	using BoundingBox2f = TBoundingBox<TVector2<float>>;
}

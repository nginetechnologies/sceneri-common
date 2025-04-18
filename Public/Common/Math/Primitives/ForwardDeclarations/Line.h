#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>
#include <Common/Math/ForwardDeclarations/Vector2.h>

namespace ngine::Math
{
	template<typename VectorType>
	struct TLine;
	using Linef = TLine<Vector3f>;
	using Line2f = TLine<Vector2f>;
}

#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>
#include <Common/Math/ForwardDeclarations/WorldCoordinate.h>

namespace ngine::Math
{
	template<typename VectorType>
	struct TSphere;
	using Spheref = TSphere<Vector3f>;
	using WorldSphere = TSphere<WorldCoordinate>;
}

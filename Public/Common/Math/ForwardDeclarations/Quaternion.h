#pragma once

#include <Common/Math/ForwardDeclarations/WorldCoordinate.h>

namespace ngine::Math
{
	template<typename T>
	struct TQuaternion;
	using Quaternionf = TQuaternion<float>;
	using Rotation3Df = Quaternionf;
	using WorldRotation = TQuaternion<WorldRotationUnitType>;
	using WorldQuaternion = TQuaternion<WorldRotationUnitType>;
}

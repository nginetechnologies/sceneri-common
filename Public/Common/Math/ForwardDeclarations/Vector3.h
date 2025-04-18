#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Math
{
	template<typename T>
	struct TVector3;

	using Vector3d = TVector3<double>;
	using Vector3f = TVector3<float>;
	using Vector3i = TVector3<int32>;
	using Vector3ui = TVector3<uint32>;

	using WorldScale = Vector3f;

	template<typename T>
	struct TBoolVector3;
	using Vector3b = TBoolVector3<uint32>;
}

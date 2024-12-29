#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Math
{
	template<typename T>
	struct TVector4;

	using Vector4r = TVector4<double>;
	using Vector4f = TVector4<float>;
	using Vector4i = TVector4<int32>;
	using Vector4ui = TVector4<uint32>;

	template<typename T>
	struct TBoolVector4;
	using Vector4b = TBoolVector4<uint32>;
}

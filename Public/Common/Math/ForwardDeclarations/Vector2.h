#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Math
{
	template<typename T>
	struct TVector2;
	using Vector2f = TVector2<float>;
	using Vector2i = TVector2<int32>;
	using Vector2ui = TVector2<uint32>;

	template<typename T>
	struct TBoolVector2;
	using Vector2b = TBoolVector2<uint32>;
}

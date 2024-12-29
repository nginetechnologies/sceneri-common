#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Math
{
	template<typename T>
	struct TRectangle;

	using Rectanglef = TRectangle<float>;
	using Rectangleui = TRectangle<uint32>;
	using Rectanglei = TRectangle<int32>;
}

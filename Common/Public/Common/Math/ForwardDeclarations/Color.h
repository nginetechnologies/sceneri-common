#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Math
{
	template<typename T>
	struct TColor;
	using Color = TColor<float>;
	using ColorByte = TColor<uint8>;
}

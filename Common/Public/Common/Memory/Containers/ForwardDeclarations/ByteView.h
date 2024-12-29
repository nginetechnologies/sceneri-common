#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	template<typename InternalByteType, typename InternalSizeType = size>
	struct TByteView;

	using ByteView = TByteView<ByteType>;
	using ConstByteView = TByteView<const ByteType>;
}

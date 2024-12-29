#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	template<typename Type, typename SizeType = size>
	struct TBitView;

	using BitView = TBitView<ByteType>;
	using ConstBitView = TBitView<const ByteType>;
}

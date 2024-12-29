#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	enum class ArrayViewFlags
	{
		Restrict = 1 << 0
	};

	template<
		typename ContainedType,
		typename InternalSizeType = uint32,
		typename InternalIndexType = InternalSizeType,
		typename InternalStoredType = ContainedType,
		uint8 Flags = 0>
	struct ArrayView;
}

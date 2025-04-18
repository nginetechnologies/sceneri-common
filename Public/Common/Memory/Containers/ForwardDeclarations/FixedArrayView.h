#pragma once

#include <Common/Memory/GetNumericSize.h>

namespace ngine
{
	template<
		typename ContainedType,
		size Size_,
		typename InternalIndexType = Memory::NumericSize<Size_>,
		typename InternalSizeType = Memory::NumericSize<Size_>,
		uint8 Flags = 0>
	struct FixedArrayView;
}

#pragma once

#include <Common/Memory/GetNumericSize.h>

namespace ngine
{
	template<typename ContainedType, size Capacity, typename SizeType = Memory::NumericSize<Capacity>, typename IndexType = SizeType>
	struct FlatVector;

	template<typename ContainedType, size Capacity, typename SizeType = Memory::NumericSize<Capacity>, typename IndexType = SizeType>
	struct FixedSizeFlatVector;
}

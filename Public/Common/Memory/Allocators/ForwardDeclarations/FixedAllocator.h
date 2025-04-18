#pragma once

#include <Common/Memory/GetNumericSize.h>

namespace ngine::Memory
{
	template<
		typename AllocatedType,
		size Capacity,
		typename SizeType_ = Memory::NumericSize<Capacity>,
		typename IndexType = SizeType_,
		size Alignment = alignof(AllocatedType)>
	struct FixedAllocator;
}

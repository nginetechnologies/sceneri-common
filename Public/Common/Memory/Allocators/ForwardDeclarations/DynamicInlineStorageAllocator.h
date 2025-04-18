#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	template<typename AllocatedType, size InlineCapacity, typename SizeType_, typename IndexType_ = SizeType_>
	struct DynamicInlineStorageAllocator;
}

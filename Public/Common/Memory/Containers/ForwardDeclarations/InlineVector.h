#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicInlineStorageAllocator.h>

namespace ngine
{
	template<typename ContainedType, size InlineCapacity, typename SizeType = uint32, typename IndexType = SizeType>
	struct InlineVector;

	template<typename ContainedType, size InlineCapacity, typename SizeType = uint32, typename IndexType = SizeType>
	struct FixedSizeInlineVector;

	template<typename ContainedType, size InlineCapacity, typename SizeType = uint32, typename IndexType = SizeType>
	struct FixedCapacityInlineVector;
}

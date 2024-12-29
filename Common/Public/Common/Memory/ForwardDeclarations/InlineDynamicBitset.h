#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicInlineStorageAllocator.h>
#include <Common/Memory/ForwardDeclarations/BitsetBase.h>
#include <Common/Math/Min.h>

namespace ngine
{
	template<size InlineCapacity, typename SizeType = uint16, typename StoredType = uint64>
	using DynamicInlineBitset = Memory::BitsetBase<
		Memory::DynamicInlineStorageAllocator<StoredType, InlineCapacity, SizeType, SizeType>,
		(uint64)Math::NumericLimits<SizeType>::Max*(uint64)8>;
}

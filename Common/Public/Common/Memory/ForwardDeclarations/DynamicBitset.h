#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicAllocator.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Memory/ForwardDeclarations/BitsetBase.h>

namespace ngine
{
	template<typename SizeType = uint16, typename StoredType = uint64>
	using DynamicBitset = Memory::
		BitsetBase<Memory::DynamicAllocator<StoredType, SizeType>, sizeof(StoredType) * 8ull * (size)Math::NumericLimits<SizeType>::Max>;
}

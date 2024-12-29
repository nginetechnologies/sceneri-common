#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/FixedAllocator.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Min.h>
#include <Common/Memory/ForwardDeclarations/BitsetBase.h>
#include <Common/Memory/GetIntegerType.h>

namespace ngine
{
	template<size Size, typename StoredType = Memory::IntegerType<Math::Min(Size, (size)64ull), false>>
	using Bitset =
		Memory::BitsetBase<Memory::FixedAllocator<StoredType, (size)Math::Ceil((float)Size / ((float)sizeof(StoredType) * 8.f))>, Size>;
}

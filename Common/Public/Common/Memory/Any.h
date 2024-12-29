#pragma once

#include <Common/Memory/ForwardDeclarations/Any.h>
#include <Common/Memory/Allocators/DynamicInlineStorageAllocator.h>
#include "AnyBase.h"

namespace ngine
{
	extern template struct TAny<Memory::DynamicAllocator<ByteType, uint32>>;
}

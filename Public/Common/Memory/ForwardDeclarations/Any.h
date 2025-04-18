#pragma once

#include "AnyBase.h"
#include <Common/Memory/Allocators/ForwardDeclarations/DynamicAllocator.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	using Any = TAny<Memory::DynamicAllocator<ByteType, uint32>>;
}

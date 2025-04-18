#pragma once

#include "AnyBase.h"
#include <Common/Memory/Allocators/ForwardDeclarations/FixedAllocator.h>

namespace ngine
{
	using Any = TAny<Memory::FixedAllocator<96, uint32>>;
}

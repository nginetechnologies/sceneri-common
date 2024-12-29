#include "Memory/Any.h"

namespace ngine
{
	template struct TAny<Memory::DynamicAllocator<ByteType, uint32>>;
}

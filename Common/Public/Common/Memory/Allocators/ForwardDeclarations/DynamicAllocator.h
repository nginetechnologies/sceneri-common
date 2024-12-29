#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	template<typename AllocatedType, typename SizeType_, typename IndexType_ = SizeType_>
	struct DynamicAllocator;
}

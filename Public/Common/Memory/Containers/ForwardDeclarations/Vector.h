#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicAllocator.h>

namespace ngine
{
	template<
		typename ContainedType,
		typename SizeType = uint32,
		typename IndexType = SizeType,
		typename AllocatorType = Memory::DynamicAllocator<ContainedType, SizeType, IndexType>>
	struct Vector;

	template<
		typename ContainedType,
		typename SizeType = uint32,
		typename IndexType = SizeType,
		typename AllocatorType = Memory::DynamicAllocator<ContainedType, SizeType, IndexType>>
	struct FixedCapacityVector;

	template<
		typename ContainedType,
		typename SizeType = uint32,
		typename IndexType = SizeType,
		typename AllocatorType = Memory::DynamicAllocator<ContainedType, SizeType, IndexType>>
	struct FixedSizeVector;
}

#pragma once

#include <Common/Memory/Allocators/FixedAllocator.h>
#include <Common/Memory/Containers/ForwardDeclarations/FlatVector.h>
#include "VectorBase.h"

namespace ngine
{
	template<typename ContainedType, size Capacity, typename SizeType, typename IndexType>
	struct FlatVector
		: public TVector<ContainedType, Memory::FixedAllocator<ContainedType, Capacity, SizeType, IndexType>, Memory::VectorFlags::AllowResize>
	{
		using BaseType =
			TVector<ContainedType, Memory::FixedAllocator<ContainedType, Capacity, SizeType, IndexType>, Memory::VectorFlags::AllowResize>;
		using BaseType::BaseType;
	};

	template<typename ContainedType, size Capacity, typename SizeType, typename IndexType>
	struct FixedSizeFlatVector
		: public TVector<ContainedType, Memory::FixedAllocator<ContainedType, Capacity, SizeType, IndexType>, Memory::VectorFlags::None>
	{
		using BaseType =
			TVector<ContainedType, Memory::FixedAllocator<ContainedType, Capacity, SizeType, IndexType>, Memory::VectorFlags::None>;
		using BaseType::BaseType;
	};
}

#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/FixedAllocator.h>
#include <Common/Function/ForwardDeclarations/FunctionPointer.h>
#include <Common/Function/ForwardDeclarations/FunctionBase.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	namespace Internal
	{
		template<typename SignatureType, size StorageSizeBytes>
		struct SelectFixedFunctionType
		{
			using Type = TFunction<SignatureType, Memory::FixedAllocator<ByteType, StorageSizeBytes>>;
		};

		template<typename SignatureType>
		struct SelectFixedFunctionType<SignatureType, 0>
		{
			using Type = FunctionPointer<SignatureType>;
		};
	}

	template<typename SignatureType, size StorageSizeBytes = 0>
	using FlatFunction = typename Internal::SelectFixedFunctionType<SignatureType, StorageSizeBytes>::Type;
}

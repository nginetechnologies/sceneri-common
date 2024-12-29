#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicInlineStorageAllocator.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Function/ForwardDeclarations/FunctionPointer.h>
#include <Common/Function/ForwardDeclarations/FunctionBase.h>

namespace ngine
{
	namespace Internal
	{
		template<typename SignatureType, size StorageSizeBytes>
		struct SelectFunctionType
		{
			using Type = TFunction<SignatureType, Memory::DynamicInlineStorageAllocator<ByteType, StorageSizeBytes, uint32>>;
		};

		template<typename SignatureType>
		struct SelectFunctionType<SignatureType, 0>
		{
			using Type = FunctionPointer<SignatureType>;
		};
	}

	template<typename SignatureType, size StorageSizeBytes = 0>
	using Function = typename Internal::SelectFunctionType<SignatureType, StorageSizeBytes>::Type;
}

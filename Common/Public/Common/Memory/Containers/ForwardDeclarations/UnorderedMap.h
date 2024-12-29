#pragma once

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicAllocator.h>

namespace absl
{
	namespace hash_internal
	{
		template<typename T>
		struct Hash;
	}

	template<typename T>
	using Hash = absl::hash_internal::Hash<T>;
}

namespace ngine
{
	namespace Memory::Internal
	{
		template<typename Type>
		struct DefaultEqualityCheck;
	}

	template<
		typename _KeyType,
		typename _ValueType,
		typename HashType = absl::Hash<_KeyType>,
		typename EqualityType = Memory::Internal::DefaultEqualityCheck<_KeyType>>
	struct UnorderedMap;
}

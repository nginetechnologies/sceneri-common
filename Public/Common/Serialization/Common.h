#pragma once

#include <Common/Memory/Allocators/Allocate.h>
#include <Common/IO/rapidjson.h>
#include <Common/Memory/Containers/ContainerCommon.h>

namespace ngine::Serialization
{
	namespace Internal
	{
		struct RapidJsonAllocator
		{
		public:
			inline static constexpr bool kNeedFree = true;
			void* Malloc(size_t size)
			{
				if (size == 0)
				{
					return nullptr;
				}

				return Memory::Allocate(size);
			}
			void* Realloc(void* originalPtr, [[maybe_unused]] size_t originalSize, size_t newSize)
			{
				if (newSize == 0)
				{
					Free(originalPtr);
					return nullptr;
				}

				if (originalPtr == nullptr)
				{
					return Memory::Allocate(newSize);
				}

				return Memory::Reallocate(originalPtr, newSize);
			}
			static void Free(void* ptr)
			{
				Memory::Deallocate(ptr);
			}
		};
	}
}

namespace rapidjson
{
	extern template class rapidjson::GenericValue<rapidjson::UTF8<>, ngine::Serialization::Internal::RapidJsonAllocator>;
	extern template class rapidjson::GenericDocument<
		rapidjson::UTF8<>,
		ngine::Serialization::Internal::RapidJsonAllocator,
		ngine::Serialization::Internal::RapidJsonAllocator>;

	extern template class rapidjson::GenericStringBuffer<rapidjson::UTF8<>, ngine::Serialization::Internal::RapidJsonAllocator>;

	extern template class rapidjson::PrettyWriter<
		rapidjson::GenericStringBuffer<rapidjson::UTF8<>, ngine::Serialization::Internal::RapidJsonAllocator>,
		rapidjson::UTF8<>,
		rapidjson::UTF8<>,
		ngine::Serialization::Internal::RapidJsonAllocator>;
}

namespace ngine::Serialization
{
	using Document = rapidjson::GenericDocument<rapidjson::UTF8<>, Internal::RapidJsonAllocator, Internal::RapidJsonAllocator>;
	using Value = rapidjson::GenericValue<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
}

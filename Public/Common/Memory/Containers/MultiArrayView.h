#pragma once

#include <Common/Memory/Align.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename... ContainedTypes>
	struct TRIVIAL_ABI MultiArrayView
	{
		inline static constexpr uint32 TypeCount = sizeof...(ContainedTypes);
	private:
		template<size ElementIndex, typename... Types>
		struct GetTypeAtIndex;

		template<size ElementIndex, typename Type_>
		struct GetTypeAtIndex<ElementIndex, Type_>
		{
			using Type = Type_;
		};

		template<size ElementIndex, typename Type_, typename... RemainingTypes>
		struct GetTypeAtIndex<ElementIndex, Type_, RemainingTypes...>
		{
			using Type = TypeTraits::Select<ElementIndex == 0, Type_, typename GetTypeAtIndex<ElementIndex - 1, RemainingTypes...>::Type>;
		};

		template<typename SearchedType, size CurrentIndex, typename... Types>
		struct GetTypeIndex;

		template<typename SearchedType, size CurrentIndex, typename Type_>
		struct GetTypeIndex<SearchedType, CurrentIndex, Type_>
		{
			inline static constexpr size Index = CurrentIndex;
		};

		template<typename SearchedType, size CurrentIndex, typename Type_, typename... RemainingTypes>
		struct GetTypeIndex<SearchedType, CurrentIndex, Type_, RemainingTypes...>
		{
			inline static constexpr size Index = TypeTraits::IsSame<SearchedType, Type_>
			                                       ? CurrentIndex
			                                       : GetTypeIndex<SearchedType, CurrentIndex + 1, RemainingTypes...>::Index;
		};
	public:
		template<size ElementIndex>
		using ElementType = typename GetTypeAtIndex<ElementIndex, ContainedTypes...>::Type;

		template<typename SearchedType>
		inline static constexpr size FirstTypeIndex = GetTypeIndex<SearchedType, 0, ContainedTypes...>::Index;

		template<typename Type>
		[[nodiscard]] FORCE_INLINE static constexpr size CalculateTypeArraySize(const size count, size offset = 0)
		{
			offset = Memory::Align(offset, alignof(Type));
			offset += sizeof(Type) * count;
			return offset;
		};
	private:
		template<size Index, size TotalCount>
		[[nodiscard]] FORCE_INLINE static constexpr size CalculateDataSize(const Array<uint32, TypeCount> itemCounts, size currentSize)
		{
			currentSize = CalculateTypeArraySize<ElementType<Index>>(itemCounts[Index], currentSize);

			if constexpr (Index + 1 < TypeCount && TotalCount > 1)
			{
				return CalculateDataSize<Index + 1, TotalCount - 1>(itemCounts, currentSize);
			}
			else
			{
				return currentSize;
			}
		}
	public:
		[[nodiscard]] FORCE_INLINE static constexpr size CalculateDataSize(const Array<uint32, TypeCount> itemCounts)
		{
			return CalculateDataSize<0, TypeCount>(itemCounts, 0);
		}

		template<size TypeIndex>
		[[nodiscard]] FORCE_INLINE static constexpr size CalculateDataOffset(const Array<uint32, TypeCount> itemCounts)
		{
			return CalculateDataSize<0, TypeIndex + 1>(itemCounts, 0) - sizeof(ElementType<TypeIndex>) * itemCounts[TypeIndex];
		}
	};
}

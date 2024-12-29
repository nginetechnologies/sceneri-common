#pragma once

#include "Bitset.h"
#include <Common/Threading/AtomicInteger.h>

namespace ngine::Threading
{
	template<size Size>
	struct AtomicBitset : protected Bitset<Size, Threading::Atomic<Memory::IntegerType<Math::Min(Size, (size)64ull), false>>>
	{
		using RawBlockType = Memory::IntegerType<Math::Min(Size, (size)64ull), false>;
		using BaseType = Bitset<Size, Threading::Atomic<RawBlockType>>;
		using BlockIndexType = typename BaseType::BlockIndexType;
		using BitIndexType = typename BaseType::BitIndexType;
		using StoredType = typename BaseType::StoredType;
		using BaseType::BaseType;
		using BaseType::operator=;

		inline static constexpr uint8 BitsPerBlock = BaseType::BitsPerBlock;

		using NonAtomicType = Bitset<Size, RawBlockType>;

		AtomicBitset(const NonAtomicType& other)
			: BaseType(reinterpret_cast<const AtomicBitset&>(other))
		{
		}
		Bitset<Size, RawBlockType> operator=(const NonAtomicType& other)
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				RawBlockType expected = block;
				const StoredType newValue = other.GetBlock(position);
				while (!block.CompareExchangeWeak(expected, newValue))
					;
				result.GetBlock(position) = expected;
			}
			return result;
		}

		using BaseType::IsSet;
		using BaseType::IsNotSet;
		using BaseType::AreAnySet;
		using BaseType::IterateSetBits;
		using BaseType::GetNumberOfSetBits;
		using BaseType::GetFirstSetIndex;
		using BaseType::GetLastSetIndex;
		using BaseType::GetNextSetIndex;
		using typename BaseType::SetBitsIterator;
		using BaseType::GetSetBitsIterator;
		using BaseType::SetAll;
		using BaseType::ClearAll;

		[[nodiscard]] operator const NonAtomicType &() const
		{
			static_assert(sizeof(NonAtomicType) == sizeof(AtomicBitset));
			static_assert(alignof(NonAtomicType) == alignof(AtomicBitset));
			return reinterpret_cast<const NonAtomicType&>(*this);
		}

		FORCE_INLINE bool Set(const BitIndexType position)
		{
			BaseType::Grow(position + 1);
			StoredType& block = BaseType::GetBlock(static_cast<BlockIndexType>(position / BitsPerBlock));
			return (block.FetchOr(RawBlockType(1ULL << position % BitsPerBlock)) & RawBlockType(1ULL << position % BitsPerBlock)) == 0;
		}

		FORCE_INLINE bool Clear(const BitIndexType position)
		{
			BaseType::Grow(position + 1);
			StoredType& block = BaseType::GetBlock(static_cast<BlockIndexType>(position / BitsPerBlock));
			return (block.FetchAnd(~RawBlockType(1ULL << position % BitsPerBlock)) & RawBlockType(1ULL << position % BitsPerBlock)) != 0;
		}

		FORCE_INLINE constexpr void Clear(const NonAtomicType other)
		{
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				block &= ~other.GetBlock(position);
			}
		}

		FORCE_INLINE constexpr void Clear()
		{
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				block = 0;
			}
		}

		[[nodiscard]] FORCE_INLINE NonAtomicType FetchClear()
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				RawBlockType expected = block;
				while (!block.CompareExchangeWeak(expected, 0))
					;
				result.GetBlock(position) = expected;
			}
			return result;
		}

		NonAtomicType operator|=(const NonAtomicType& other)
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = block.FetchOr(other.GetBlock(position));
			}
			return result;
		}
		NonAtomicType operator|(const NonAtomicType& other) const
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = block.Load() | other.GetBlock(position);
			}
			return result;
		}
		NonAtomicType operator&=(const NonAtomicType& other)
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = block.FetchAnd(other.GetBlock(position));
			}
			return result;
		}
		NonAtomicType operator&(const NonAtomicType& other) const
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				const StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = block.Load() & other.GetBlock(position);
			}
			return result;
		}
		NonAtomicType operator^=(const NonAtomicType& other)
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				const StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = block.FetchXor(other.GetBlock(position));
			}
			return result;
		}
		NonAtomicType operator^(const NonAtomicType& other) const
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				const StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = block.Load() ^ other.GetBlock(position);
			}
			return result;
		}
		NonAtomicType operator~() const
		{
			NonAtomicType result;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				const StoredType& block = BaseType::GetBlock(position);
				result.GetBlock(position) = ~block.Load();
			}
			return result;
		}

		[[nodiscard]] bool AreAnySet(const NonAtomicType& other) const
		{
			typename NonAtomicType::RawBlockType result = 0;
			for (BlockIndexType position = 0, count = BaseType::m_allocator.GetCapacity(); position < count; ++position)
			{
				const StoredType& block = BaseType::GetBlock(position);
				result |= block.Load() & other.GetBlock(position);
			}
			return result != 0;
		}
	};
}

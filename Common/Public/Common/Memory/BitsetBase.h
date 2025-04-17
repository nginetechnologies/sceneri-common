#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Range.h>
#include <Common/Memory/GetNumericSize.h>
#include <Common/Memory/CountBits.h>
#include <Common/Memory/GetIntegerType.h>
#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	namespace Memory
	{
		template<typename AllocatorType_, uint64 MaximumCount>
		struct TRIVIAL_ABI BitsetBase
		{
			using AllocatorType = AllocatorType_;
			using StoredType = typename AllocatorType::ElementType;

			using BlockIndexType = typename AllocatorType::IndexType;
			inline static constexpr uint16 BitsPerBlock = sizeof(StoredType) * 8;
			using RawBlockType = Memory::IntegerType<BitsPerBlock, false>;
			inline static constexpr uint64 TheoreticalMaximumCapacity = (uint64)AllocatorType::GetTheoreticalCapacity() * (uint64)BitsPerBlock;
			static_assert(MaximumCount <= TheoreticalMaximumCapacity);
			inline static constexpr uint64 MaximumCapacity = MaximumCount;
			inline static constexpr uint64 MaximumIndex = MaximumCount - 1;
			using BitIndexType = Memory::NumericSize<MaximumCount>;

			using BlockView = typename AllocatorType::View;
			using ConstBlockView = typename AllocatorType::ConstView;
			using RestrictedBlockView = typename AllocatorType::RestrictedView;
			using ConstRestrictedBlockView = typename AllocatorType::ConstRestrictedView;
			using DynamicBlockView = ArrayView<StoredType, BlockIndexType>;
			using ConstDynamicBlockView = ArrayView<const StoredType, BlockIndexType>;
			using DynamicRestrictedBlockView = ArrayView<StoredType, BlockIndexType, BlockIndexType, StoredType, (uint8)ArrayViewFlags::Restrict>;
			using ConstRestrictedDynamicBlockView =
				ArrayView<const StoredType, BlockIndexType, BlockIndexType, const StoredType, (uint8)ArrayViewFlags::Restrict>;

			BitsetBase()
			{
				m_allocator.GetRestrictedView().ZeroInitialize();
			}
			explicit BitsetBase(const Memory::ConstructWithSizeType, const Memory::ZeroedType, const BlockIndexType size) noexcept
				: m_allocator(Memory::Reserve, size)
			{
				m_allocator.GetRestrictedView().ZeroInitialize();
			}
			explicit BitsetBase(const Memory::ReserveType, const BlockIndexType size) noexcept
				: m_allocator(Memory::Reserve, size)
			{
				m_allocator.GetRestrictedView().ZeroInitialize();
			}
			BitsetBase(const Memory::SetAllType)
			{
				m_allocator.GetRestrictedView().ZeroInitialize();
				SetAll();
			}
			BitsetBase(const BitsetBase& __restrict other)
				: m_allocator(Memory::Reserve, other.m_allocator.GetCapacity())
			{
				if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
				{
					m_allocator.GetView().CopyFrom(other.m_allocator.GetView());
				}
				else
				{
					const RestrictedBlockView targetView = m_allocator.GetRestrictedView();
					const ConstRestrictedBlockView sourceView = other.m_allocator.GetRestrictedView();
					for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
					{
						targetView[position] = sourceView[position];
					}
				}
			}
			template<typename OtherAllocatorType, uint64 OtherMaximumCount>
			BitsetBase(const BitsetBase<OtherAllocatorType, OtherMaximumCount>& __restrict other)
				: m_allocator(Memory::Reserve, Math::Min(AllocatorType::GetTheoreticalCapacity(), other.m_allocator.GetCapacity()))
			{
				using OtherBitsetType = BitsetBase<OtherAllocatorType, OtherMaximumCount>;
				using OtherStoredType = typename OtherBitsetType::StoredType;
				if constexpr (TypeTraits::IsTriviallyCopyable<StoredType> && TypeTraits::IsTriviallyCopyable<OtherStoredType>)
				{
					BlockView targetView = m_allocator.GetView();
					typename OtherBitsetType::ConstBlockView sourceView = other.m_allocator.GetView();
					const BlockIndexType blockCount = BlockIndexType(Math::Min(targetView.GetSize(), sourceView.GetSize()));

					DynamicBlockView remainingView = targetView.GetSubView(blockCount, targetView.GetSize());
					DynamicBlockView dynamicTargetView = targetView.GetSubView(0, blockCount);
					typename OtherBitsetType::ConstDynamicBlockView dynamicSourceView = sourceView.GetSubView(0, blockCount);

					dynamicTargetView.CopyFrom(dynamicSourceView);
					remainingView.ZeroInitialize();
				}
				else
				{
					const RestrictedBlockView targetView = m_allocator.GetRestrictedView();
					const typename OtherBitsetType::ConstRestrictedBlockView sourceView = other.m_allocator.GetRestrictedView();
					const BlockIndexType blockCount = Math::Min(targetView.GetSize(), sourceView.GetSize());

					for (BlockIndexType position = 0; position < blockCount; ++position)
					{
						targetView[position] = sourceView[position];
					}

					// Reset remainder
					for (BlockIndexType position = blockCount, count = m_allocator.GetCapacity(); position < count; ++position)
					{
						targetView[position] = 0;
					}
				}
			}
			BitsetBase& operator=(const BitsetBase& __restrict other)
			{
				m_allocator.Allocate(other.m_allocator.GetCapacity());

				if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
				{
					m_allocator.GetView().CopyFrom(other.m_allocator.GetView());
				}
				else
				{
					const RestrictedBlockView targetView = m_allocator.GetRestrictedView();
					const ConstRestrictedBlockView sourceView = other.m_allocator.GetRestrictedView();
					for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
					{
						targetView[position] = sourceView[position];
					}
				}
				return *this;
			}

			void Reserve(const BitIndexType bitCount)
			{
				Grow(bitCount);
			}

			[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr static BlockIndexType GetBitBlockIndex(const BitIndexType bitIndex)
			{
				return static_cast<BlockIndexType>(bitIndex / BitsPerBlock);
			}

			[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr static BitIndexType GetBlockBitIndex(const BitIndexType bitIndex)
			{
				return bitIndex % BitsPerBlock;
			}

			[[nodiscard]] PURE_STATICS constexpr bool IsSet(const BitIndexType position) const
			{
				return ((m_allocator.GetRestrictedView()[GetBitBlockIndex(position)] & (1ULL << GetBlockBitIndex(position))) != 0);
			}
			[[nodiscard]] PURE_STATICS constexpr bool IsNotSet(const BitIndexType position) const
			{
				return ((m_allocator.GetRestrictedView()[GetBitBlockIndex(position)] & (1ULL << GetBlockBitIndex(position))) == 0);
			}

			[[nodiscard]] PURE_STATICS constexpr bool AreNoneSet() const
			{
				if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
				{
					BitsetBase zeroed;
					zeroed.Reserve(BitIndexType(m_allocator.GetCapacity() * BitsPerBlock));
					return Memory::Compare(m_allocator.GetData(), zeroed.m_allocator.GetData(), m_allocator.GetCapacity() * sizeof(StoredType)) == 0;
				}
				else
				{
					for (const StoredType& element : m_allocator.GetRestrictedView())
					{
						if ((RawBlockType)element == 0)
						{
							return false;
						}
					}
					return true;
				}
			}

			[[nodiscard]] PURE_STATICS constexpr bool AreAllSet() const
			{
				return GetNumberOfSetBits() == MaximumCount;
			}
			[[nodiscard]] PURE_STATICS constexpr bool AreAnyNotSet() const
			{
				return GetNumberOfSetBits() != MaximumCount;
			}

			[[nodiscard]] PURE_STATICS constexpr bool AreNoneSet(const BitsetBase& __restrict other) const
			{
				bool areNoneSet{true};
				const ConstRestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					areNoneSet &= (blockView[position] & otherRestrictedBlockView[position]) == 0;
				}
				return areNoneSet;
			}

			[[nodiscard]] PURE_STATICS constexpr bool AreAnySet() const
			{
				return !AreNoneSet();
			}

			[[nodiscard]] PURE_STATICS constexpr bool AreAnySet(const BitsetBase& __restrict other) const
			{
				bool areAnySet{false};
				const ConstRestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					areAnySet |= (blockView[position] & otherRestrictedBlockView[position]) != 0;
				}
				return areAnySet;
			}

			[[nodiscard]] PURE_STATICS constexpr bool AreAllSet(const BitsetBase& __restrict other) const
			{
				bool areAllSet = true;
				const ConstRestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					areAllSet &= ((blockView[position] & otherRestrictedBlockView[position]) == otherRestrictedBlockView[position]);
				}
				return areAllSet;
			}
			[[nodiscard]] PURE_STATICS constexpr bool AreAnyNotSet(const BitsetBase& __restrict other) const
			{
				bool areAnyNotSet = false;
				const ConstRestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					areAnyNotSet |= (((blockView[position] ^ otherRestrictedBlockView[position]) & otherRestrictedBlockView[position]) != 0);
				}
				return areAnyNotSet;
			}

			PURE_STATICS BitsetBase operator&(const BitsetBase& __restrict other) const
			{
				BitsetBase result = *this;
				const RestrictedBlockView blockView = result.m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] &= otherRestrictedBlockView[position];
				}
				return result;
			}

			BitsetBase& operator&=(const BitsetBase& __restrict other)
			{
				Grow(other.m_allocator.GetCapacity());
				const RestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] &= otherRestrictedBlockView[position];
				}
				return *this;
			}

			PURE_STATICS BitsetBase operator|(const BitsetBase& __restrict other) const
			{
				BitsetBase result = *this;
				const RestrictedBlockView blockView = result.m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] |= otherRestrictedBlockView[position];
				}
				return result;
			}

			BitsetBase& operator|=(const BitsetBase& __restrict other)
			{
				Grow(other.m_allocator.GetCapacity());
				const RestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] |= otherRestrictedBlockView[position];
				}
				return *this;
			}

			PURE_STATICS BitsetBase operator^(const BitsetBase& __restrict other) const
			{
				BitsetBase result = *this;
				const RestrictedBlockView blockView = result.m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] ^= otherRestrictedBlockView[position];
				}
				return result;
			}

			BitsetBase& operator^=(const BitsetBase& __restrict other) noexcept
			{
				Grow(other.m_allocator.GetCapacity());
				const RestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] ^= otherRestrictedBlockView[position];
				}
				return *this;
			}

			[[nodiscard]] PURE_STATICS BitsetBase operator~() const noexcept
			{
				BitsetBase bitset = *this;
				const RestrictedBlockView blockView = bitset.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] = ~blockView[position];
				}
				return bitset;
			}

			[[nodiscard]] PURE_STATICS bool operator==(const BitsetBase& __restrict other) const noexcept
			{
				if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
				{
					if (m_allocator.GetCapacity() == other.m_allocator.GetCapacity())
					{
						return Memory::Compare(m_allocator.GetData(), other.m_allocator.GetData(), m_allocator.GetCapacity() * sizeof(StoredType)) == 0;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return m_allocator.GetRestrictedView() == other.m_allocator.GetRestrictedView();
				}
			}

			[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const BitsetBase& __restrict other) const noexcept
			{
				return !(*this == other);
			}

			constexpr void Set(const BitIndexType position)
			{
				Grow(position + 1);
				m_allocator.GetRestrictedView()[GetBitBlockIndex(position)] |= 1ULL << GetBlockBitIndex(position);
			}

			constexpr void Toggle(const BitIndexType position)
			{
				Grow(position + 1);
				m_allocator.GetRestrictedView()[GetBitBlockIndex(position)] ^= 1ULL << GetBlockBitIndex(position);
			}

			constexpr void Clear(const BitIndexType position)
			{
				Grow(position + 1);
				m_allocator.GetRestrictedView()[GetBitBlockIndex(position)] &= ~(1ULL << GetBlockBitIndex(position));
			}

			constexpr void Clear(const BitsetBase& __restrict other)
			{
				Grow(other.m_allocator.GetCapacity());

				const RestrictedBlockView blockView = m_allocator.GetRestrictedView();
				const ConstRestrictedBlockView otherRestrictedBlockView = other.m_allocator.GetRestrictedView();
				for (BlockIndexType position = 0, count = m_allocator.GetCapacity(); position < count; ++position)
				{
					blockView[position] &= ~otherRestrictedBlockView[position];
				}
			}

			void SetAll(const Math::Range<BitIndexType> range)
			{
				Grow(range.GetMinimum() + range.GetSize());

				const BitIndexType firstBitIndex = range.GetMinimum();
				const BitIndexType lastBitIndex = range.GetMaximum();

				const BlockIndexType firstBlockIndex = GetBitBlockIndex(firstBitIndex);
				const BitIndexType firstBlockBitIndex = GetBlockBitIndex(firstBitIndex);

				const BlockIndexType lastBlockIndex = GetBitBlockIndex(lastBitIndex);
				const BitIndexType lastBlockBitIndex = GetBlockBitIndex(lastBitIndex);

				const BlockIndexType firstBlockEndBitIndex = (BlockIndexType)Math::Min(firstBlockBitIndex + range.GetSize(), BitsPerBlock);
				const bool isFirstBlockIncomplete = (firstBlockEndBitIndex - firstBlockBitIndex) < BitsPerBlock;
				const BlockIndexType firstCompleteBlockIndex = firstBlockIndex + isFirstBlockIncomplete;

				const bool isLastBlockIncomplete = lastBlockBitIndex != (BitsPerBlock - 1);
				const BlockIndexType lastCompleteBlockIndex = Math::Min(BlockIndexType(lastBlockIndex - isLastBlockIncomplete), lastBlockIndex);

				const RestrictedBlockView data = m_allocator.GetRestrictedView();

				// Handle the first block if it isn't fully covered by the range
				if (isFirstBlockIncomplete)
				{
					RawBlockType rawBlock{0};
					for (BitIndexType index = firstBlockBitIndex; index < firstBlockEndBitIndex; ++index)
					{
						rawBlock |= (1ULL << index);
					}
					data[firstBlockIndex] |= rawBlock;
				}

				if (firstCompleteBlockIndex <= lastCompleteBlockIndex)
				{
					const DynamicRestrictedBlockView completeBlockView =
						data.GetSubView(firstCompleteBlockIndex, (lastCompleteBlockIndex - firstCompleteBlockIndex) + 1);

					if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
					{
						// Set all the complete blocks using Memory::Set (likely memset)
						completeBlockView.OneInitialize();
					}
					else
					{
						for (StoredType& block : completeBlockView)
						{
							block = RawBlockType(Math::NumericLimits<RawBlockType>::Max);
						}
					}
				}

				if (isLastBlockIncomplete && firstBlockIndex != lastBlockIndex)
				{
					RawBlockType rawBlock{0};
					for (BitIndexType index = 0; index < (lastBlockBitIndex + 1); ++index)
					{
						rawBlock |= 1ULL << index;
					}
					data[lastBlockIndex] |= rawBlock;
				}
			}
			void SetAll()
			{
				if constexpr (MaximumCapacity == TheoreticalMaximumCapacity)
				{
					if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
					{
						m_allocator.GetRestrictedView().OneInitialize();
					}
					else
					{
						for (StoredType& element : m_allocator.GetRestrictedView())
						{
							element = Math::NumericLimits<RawBlockType>::Max;
						}
					}
				}
				else if (MaximumCapacity == m_allocator.GetCapacity())
				{
					if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
					{
						m_allocator.GetRestrictedView().OneInitialize();
					}
					else
					{
						for (StoredType& element : m_allocator.GetRestrictedView())
						{
							element = Math::NumericLimits<RawBlockType>::Max;
						}
					}
				}
				else
				{
					SetAll(Math::Range<BitIndexType>::Make(0, MaximumCapacity));
				}
			}

			void ClearAll(const Math::Range<BitIndexType> range)
			{
				Grow(range.GetMinimum() + range.GetSize());

				const BitIndexType firstBitIndex = range.GetMinimum();
				const BitIndexType lastBitIndex = range.GetMaximum();

				const BlockIndexType firstBlockIndex = GetBitBlockIndex(firstBitIndex);
				const BitIndexType firstBlockBitIndex = GetBlockBitIndex(firstBitIndex);

				const BlockIndexType lastBlockIndex = GetBitBlockIndex(lastBitIndex);
				const BitIndexType lastBlockBitIndex = GetBlockBitIndex(lastBitIndex);

				const BlockIndexType firstBlockEndBitIndex = (BlockIndexType)Math::Min(firstBlockBitIndex + range.GetSize(), BitsPerBlock);
				const bool isFirstBlockIncomplete = (firstBlockEndBitIndex - firstBlockBitIndex) < BitsPerBlock;
				const BlockIndexType firstCompleteBlockIndex = firstBlockIndex + isFirstBlockIncomplete;

				const bool isLastBlockIncomplete = lastBlockBitIndex != (BitsPerBlock - 1);
				const BlockIndexType lastCompleteBlockIndex = Math::Min(BlockIndexType(lastBlockIndex - isLastBlockIncomplete), lastBlockIndex);

				const RestrictedBlockView data = m_allocator.GetRestrictedView();

				// Handle the first block if it isn't fully covered by the range
				if (isFirstBlockIncomplete)
				{
					RawBlockType rawBlock{Math::NumericLimits<RawBlockType>::Max};
					for (BitIndexType index = firstBlockBitIndex; index < firstBlockEndBitIndex; ++index)
					{
						rawBlock &= ~(1ULL << index);
					}
					data[firstBlockIndex] &= rawBlock;
				}

				if (firstCompleteBlockIndex <= lastCompleteBlockIndex)
				{
					const DynamicRestrictedBlockView completeBlockView =
						data.GetSubView(firstCompleteBlockIndex, (lastCompleteBlockIndex - firstCompleteBlockIndex) + 1);

					if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
					{
						// Clear all the complete blocks using Memory::Set (likely memset)
						completeBlockView.ZeroInitialize();
					}
					else
					{
						for (StoredType& block : completeBlockView)
						{
							block = RawBlockType(0);
						}
					}
				}

				if (isLastBlockIncomplete && firstBlockIndex != lastBlockIndex)
				{
					RawBlockType rawBlock{0};
					for (BitIndexType index = 0; index < (lastBlockBitIndex + 1); ++index)
					{
						rawBlock |= 1ULL << index;
					}
					data[lastBlockIndex] &= ~rawBlock;
				}
			}
			void ClearAll()
			{
				if constexpr (MaximumCapacity == TheoreticalMaximumCapacity)
				{
					if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
					{
						m_allocator.GetRestrictedView().ZeroInitialize();
					}
					else
					{
						for (StoredType& element : m_allocator.GetRestrictedView())
						{
							element = (RawBlockType)0;
						}
					}
				}
				else if (MaximumCapacity == m_allocator.GetCapacity())
				{
					if constexpr (TypeTraits::IsTriviallyCopyable<StoredType>)
					{
						m_allocator.GetRestrictedView().ZeroInitialize();
					}
					else
					{
						for (StoredType& element : m_allocator.GetRestrictedView())
						{
							element = (RawBlockType)0;
						}
					}
				}
				else
				{
					ClearAll(Math::Range<BitIndexType>::Make(0, MaximumCapacity));
				}
			}

			[[nodiscard]] inline PURE_STATICS BitIndexType GetNumberOfSetBits() const
			{
				const ConstRestrictedBlockView data = m_allocator.GetRestrictedView();

				BitIndexType numberOfSetBits = 0;

				for (const StoredType& __restrict value : data)
				{
					numberOfSetBits += (BitIndexType)Memory::GetNumberOfSetBits((RawBlockType)value);
				}

				return numberOfSetBits;
			}

			[[nodiscard]] PURE_STATICS BitIndex<BitIndexType> GetFirstSetIndex() const
			{
				const ConstRestrictedBlockView data = m_allocator.GetRestrictedView();

				BitIndexType indexOffset = 0;

				for (const StoredType& __restrict value : data)
				{
					if (const BitIndex<RawBlockType> index = Memory::GetFirstSetIndex((RawBlockType)value))
					{
						return BitIndex<BitIndexType>{BitIndexType((BitIndexType)index.m_value + indexOffset)};
					}

					indexOffset += BitsPerBlock;
				}

				// Invalid
				return {0};
			}

			[[nodiscard]] PURE_STATICS BitIndex<BitIndexType> GetLastSetIndex() const
			{
				const ConstRestrictedBlockView data = m_allocator.GetRestrictedView();

				BitIndexType indexOffset = BitsPerBlock * (m_allocator.GetCapacity() - 1);

				for (auto it = data.end() - 1, endIt = data.begin() - 1; it != endIt; --it)
				{
					const StoredType& __restrict value = *it;
					if (const BitIndex<RawBlockType> index = Memory::GetLastSetIndex((RawBlockType)value))
					{
						return BitIndex<BitIndexType>{BitIndexType(indexOffset + (BitIndexType)index.m_value)};
					}

					indexOffset -= BitsPerBlock;
				}

				// Invalid
				return {0};
			}

			[[nodiscard]] PURE_STATICS Optional<BitIndexType> GetNextSetIndex(const BitIndexType startIndex) const
			{
				ConstRestrictedDynamicBlockView data = m_allocator.GetRestrictedView() + GetBitBlockIndex(startIndex);

				BitIndexType indexOffset = startIndex;
				if (BitIndex<RawBlockType> firstBlockSetIndex = Memory::GetFirstSetIndex(static_cast<RawBlockType>(data[0] >> GetBlockBitIndex(startIndex))))
				{
					Assert(IsSet(BitIndexType(*firstBlockSetIndex + indexOffset)));
					return BitIndexType(*firstBlockSetIndex + indexOffset);
				}
				data++;
				indexOffset += BitsPerBlock - GetBlockBitIndex(startIndex);

				for (const StoredType& __restrict value : data)
				{
					if (const BitIndex<RawBlockType> index = Memory::GetFirstSetIndex((RawBlockType)value))
					{
						Assert(IsSet(BitIndexType(*index + indexOffset)));
						return BitIndexType(*index + indexOffset);
					}

					indexOffset += BitsPerBlock;
				}

				return Invalid;
			}

			struct SetBitsIterator
			{
				SetBitsIterator() = default;
				constexpr SetBitsIterator(const ConstRestrictedDynamicBlockView view, const BitIndexType bitIndex = 0)
					: m_view(view)
					, m_bitIndex(bitIndex)
				{
				}

				struct Iterator
				{
					[[nodiscard]] FORCE_INLINE bool operator<(const Iterator& other) const
					{
						return m_bitIndex < other.m_bitIndex;
					}
					[[nodiscard]] FORCE_INLINE bool operator==(const Iterator& other) const
					{
						return m_bitIndex == other.m_bitIndex;
					}
					[[nodiscard]] FORCE_INLINE bool operator!=(const Iterator& other) const
					{
						return !operator==(other);
					}

					inline void operator++()
					{
						ConstRestrictedDynamicBlockView view = m_view;
						BitIndexType bitIndex = m_bitIndex;
						typename Memory::SetBitsIterator<RawBlockType>::Iterator currentBlockIterator = m_currentBlockIterator;
						++currentBlockIterator;
						while (!currentBlockIterator.IsSet())
						{
							view++;
							if (view.HasElements())
							{
								currentBlockIterator = Memory::GetSetBitsIterator((RawBlockType)view[0]).begin();
								bitIndex += BitsPerBlock;
							}
							else
							{
								// Fast forward to end
								m_view = {};
								m_bitIndex = (BitIndexType)Math::Min(bitIndex + BitsPerBlock - GetBlockBitIndex(bitIndex), (BitIndexType)MaximumCount);
								m_currentBlockIterator = {};
								return;
							}
						}
						m_view = view;
						m_bitIndex = bitIndex;
						m_currentBlockIterator = currentBlockIterator;
					}

					void operator+=(BitIndexType offset)
					{
						ConstRestrictedDynamicBlockView view = m_view;
						BitIndexType bitIndex = m_bitIndex;
						typename Memory::SetBitsIterator<RawBlockType>::Iterator currentBlockIterator = m_currentBlockIterator;

						while (offset > 0)
						{
							++currentBlockIterator;
							while (!currentBlockIterator.IsSet())
							{
								view++;
								if (view.HasElements())
								{
									currentBlockIterator = Memory::GetSetBitsIterator((RawBlockType)view[0]).begin();
									bitIndex += BitsPerBlock;
								}
								else
								{
									// Fast forward to end
									m_view = {};
									m_bitIndex = (BitIndexType)Math::Min(bitIndex + BitsPerBlock - GetBlockBitIndex(bitIndex), (BitIndexType)MaximumCount);
									m_currentBlockIterator = {};
									return;
								}
							}
							--offset;
						}

						m_view = view;
						m_bitIndex = bitIndex;
						m_currentBlockIterator = currentBlockIterator;
					}

					Iterator operator+(const BitIndexType offset) const
					{
						Iterator newIterator = *this;
						newIterator += offset;
						return newIterator;
					}

					[[nodiscard]] FORCE_INLINE BitIndexType operator*() const
					{
						return m_bitIndex + *m_currentBlockIterator;
					}

					ConstRestrictedDynamicBlockView m_view;
					BitIndexType m_bitIndex = 0;
					typename Memory::SetBitsIterator<RawBlockType>::Iterator m_currentBlockIterator;
				};

				[[nodiscard]] constexpr inline Iterator begin() const
				{
					ConstRestrictedDynamicBlockView view = m_view;
					if (view.HasElements())
					{
						BitIndexType index = 0;
						const BitIndexType bitIndex = m_bitIndex;
						const BitIndexType firstBlockBitIndex = GetBlockBitIndex(bitIndex);

						RawBlockType currentBlock = (RawBlockType)view[0] >> firstBlockBitIndex;
						Memory::SetBitsIterator<RawBlockType> bitIterator(currentBlock);
						typename Memory::SetBitsIterator<RawBlockType>::Iterator it = bitIterator.begin();
						while (it == bitIterator.end() && view.GetSize() > 1)
						{
							view++;
							currentBlock = (RawBlockType)view[0];
							bitIterator = currentBlock;
							it = bitIterator.begin();
							index += BitsPerBlock;
						}

						if (it != bitIterator.end())
						{
							return {view, static_cast<BitIndexType>(index + bitIndex), Memory::GetSetBitsIterator(currentBlock).begin()};
						}
						else
						{
							return end();
						}
					}
					else
					{
						return end();
					}
				}

				[[nodiscard]] constexpr Iterator end() const
				{
					ConstRestrictedDynamicBlockView view = m_view;
					const BlockIndexType blockIndexOffset = GetBitBlockIndex(m_bitIndex);
					return {
						{},
						(BitIndexType)Math::Min((uint64)((view.GetSize() + blockIndexOffset) * BitsPerBlock), MaximumCount),
						Memory::GetSetBitsIterator(view.HasElements() ? (RawBlockType)view.GetLastElement() : (RawBlockType)0).end()
					};
				}
			protected:
				ConstRestrictedDynamicBlockView m_view;
				BitIndexType m_bitIndex = 0;
			};

			struct SetBitsReverseIterator
			{
				constexpr SetBitsReverseIterator(const ConstRestrictedDynamicBlockView view, const BitIndexType bitIndex = 0)
					: m_view(view)
					, m_bitIndex(bitIndex)
				{
				}

				struct Iterator
				{
					[[nodiscard]] FORCE_INLINE bool operator==(const Iterator& other) const
					{
						return m_currentBlockIterator == other.m_currentBlockIterator;
					}
					[[nodiscard]] FORCE_INLINE bool operator!=(const Iterator& other) const
					{
						return !operator==(other);
					}

					inline void operator++()
					{
						ConstRestrictedDynamicBlockView view = m_view;
						BitIndexType bitIndex = m_bitIndex;
						typename Memory::SetBitsReverseIterator<RawBlockType>::Iterator currentBlockIterator = m_currentBlockIterator;

						++currentBlockIterator;
						while (!currentBlockIterator.IsSet() && view.GetSize() > 1)
						{
							view--;
							currentBlockIterator = Memory::GetSetBitsReverseIterator((RawBlockType)view.GetLastElement()).begin();
							bitIndex -= BitsPerBlock;
						}

						m_view = view;
						m_bitIndex = bitIndex;
						m_currentBlockIterator = currentBlockIterator;
					}

					void operator+=(BitIndexType offset)
					{
						ConstRestrictedDynamicBlockView view = m_view;
						BitIndexType bitIndex = m_bitIndex;
						typename Memory::SetBitsIterator<RawBlockType>::Iterator currentBlockIterator = m_currentBlockIterator;

						while (offset > 0)
						{
							++m_currentBlockIterator;
							while (!currentBlockIterator.IsSet() && view.GetSize() > 1)
							{
								view--;
								currentBlockIterator = Memory::GetSetBitsReverseIterator((RawBlockType)view.GetLastElement()).begin();
								bitIndex -= BitsPerBlock;
							}
							--offset;
						}

						m_view = view;
						m_bitIndex = bitIndex;
						m_currentBlockIterator = currentBlockIterator;
					}

					[[nodiscard]] FORCE_INLINE Iterator operator+(const BitIndexType offset)
					{
						Iterator newIterator = *this;
						newIterator += offset;
						return newIterator;
					}

					[[nodiscard]] FORCE_INLINE BitIndexType operator*() const
					{
						return m_bitIndex + *m_currentBlockIterator;
					}

					ConstRestrictedDynamicBlockView m_view;
					BitIndexType m_bitIndex = 0;
					typename Memory::SetBitsReverseIterator<RawBlockType>::Iterator m_currentBlockIterator;
				};

				[[nodiscard]] constexpr inline Iterator begin() const
				{
					ConstRestrictedDynamicBlockView view = m_view;
					BitIndexType index = (view.GetSize() - 1) * BitsPerBlock;

					Memory::SetBitsReverseIterator<RawBlockType> bitIterator((RawBlockType)view.GetLastElement());
					typename Memory::SetBitsReverseIterator<RawBlockType>::Iterator it = bitIterator.begin();
					while (it == bitIterator.end() && view.GetSize() > 1)
					{
						view--;
						bitIterator = (RawBlockType)view.GetLastElement();
						it = bitIterator.begin();
index -= BitsPerBlock;
					}

					return {
						view,
						static_cast<BitIndexType>(index + m_bitIndex),
						Memory::GetSetBitsReverseIterator((RawBlockType)view.GetLastElement()).begin()
					};
				}

				[[nodiscard]] constexpr FORCE_INLINE Iterator end() const
				{
					return {{}, static_cast<BitIndexType>(m_view.GetSize() + m_bitIndex), {}};
				}
			protected:
				ConstRestrictedDynamicBlockView m_view;
				BitIndexType m_bitIndex;
			};
			
			[[nodiscard]] SetBitsIterator GetSetBitsIterator(const BitIndexType startIndex, const BitIndexType count) const
			{
				return SetBitsIterator{
					m_allocator.GetRestrictedView()
						.GetSubView(GetBitBlockIndex(startIndex), static_cast<BlockIndexType>(Math::Ceil((float)count / (float)BitsPerBlock))),
					startIndex
				};
			}

			[[nodiscard]] SetBitsIterator GetSetBitsIterator() const
			{
				return SetBitsIterator{m_allocator.GetRestrictedView()};
			}

			[[nodiscard]] SetBitsReverseIterator GetSetBitsReverseIterator() const
			{
				return SetBitsReverseIterator{m_allocator.GetRestrictedView()};
			}

			[[nodiscard]] ConstRestrictedBlockView GetRestrictedBlockView() const
			{
				return m_allocator.GetRestrictedView();
			}

			// TODO: Need to replace this with an iterator
			template<typename Callback>
			void IterateSetBits(Callback&& callback) const
			{
				BitIndexType indexOffset = 0;

				for (const StoredType& __restrict value : m_allocator.GetRestrictedView())
				{
					for (const RawBlockType bitIndex : Memory::GetSetBitsIterator((RawBlockType)value))
					{
						callback(static_cast<BitIndexType>(indexOffset + (BitIndexType)bitIndex));
					}

					indexOffset += BitsPerBlock;
				}
			}

			template<typename Callback>
			void IterateUnsetBits(Callback callback) const
			{
				BitIndexType indexOffset = 0;

				for (const StoredType& __restrict value : m_allocator.GetRestrictedView())
				{
					for (const RawBlockType bitIndex : Memory::GetUnsetBitsIterator((RawBlockType)value))
					{
						callback(static_cast<BitIndexType>(indexOffset + (BitIndexType)bitIndex));
					}

					indexOffset += BitsPerBlock;
				}
			}

			template<typename Callback>
			inline void IterateSetBitRanges(Callback&& callback)
			{
				BitIndexType firstIndex;
				if (const BitIndex<BitIndexType> firstSetIndex = GetFirstSetIndex())
				{
					firstIndex = *firstSetIndex;
				}
				else
				{
					return;
				}

				const ConstRestrictedBlockView changedInstancesBlocks = GetRestrictedBlockView();
				const BlockIndexType firstBlockIndex = GetBitBlockIndex(firstIndex);
				auto blockIt = changedInstancesBlocks.begin() + firstBlockIndex;
				auto blockEnd = changedInstancesBlocks.end();

				BitIndexType bitIndex = GetBlockBitIndex(firstIndex);
				for (; blockIt != blockEnd;)
				{
					RawBlockType bitsetBlock = (RawBlockType)*blockIt;
					bitsetBlock >>= bitIndex;

					BitIndexType setBitCount = (BitIndexType)Memory::GetNumberOfTrailingZeros(~(RawBlockType)bitsetBlock);

					bitIndex += setBitCount;
					bitsetBlock >>= setBitCount;

					if (setBitCount != BitsPerBlock && bitIndex != BitsPerBlock)
					{
						const BlockIndexType indexOffset = changedInstancesBlocks.GetIteratorIndex(blockIt) * BitsPerBlock;
						Assert(firstIndex != indexOffset + bitIndex);
						callback(Math::Range<BitIndexType>::MakeStartToEnd(firstIndex, indexOffset + bitIndex - 1));

						if (bitsetBlock & 1)
						{
							firstIndex = indexOffset + bitIndex;
						}
						else
						{
							// Skip zeroes in this block first
							BitIndexType skippedBitCount = (BitIndexType)Memory::GetNumberOfTrailingZeros(bitsetBlock);
							bitIndex += skippedBitCount;
							bitsetBlock >>= skippedBitCount;
							if (bitsetBlock & 1)
							{
								firstIndex = indexOffset + bitIndex;
								Assert((*blockIt >> bitIndex) & 1);
								continue;
							}

							// Find the next set block first index
							if (const Optional<BitIndexType> nextSetIndex = GetNextSetIndex(indexOffset + BitsPerBlock))
							{
								firstIndex = *nextSetIndex;
								Assert(IsSet(firstIndex));
								const BitIndexType newBlockIndex = GetBitBlockIndex(firstIndex);
								bitIndex = GetBlockBitIndex(firstIndex);
								blockIt = changedInstancesBlocks.begin() + newBlockIndex;
								Assert(*blockIt != 0);
								Assert((*blockIt >> bitIndex) & 1);
								continue;
							}
							else
							{
								break;
							}
						}
					}

					bitIndex = 0;
					++blockIt;
				}
			}

			[[nodiscard]] FORCE_INLINE PURE_STATICS StoredType& GetBlock(const BlockIndexType index)
			{
				return m_allocator.GetRestrictedView()[index];
			}
			[[nodiscard]] FORCE_INLINE PURE_STATICS const StoredType& GetBlock(const BlockIndexType index) const
			{
				return m_allocator.GetRestrictedView()[index];
			}

			[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
			{
				return MaximumCount;
			}
			bool Compress(BitView& target) const;
			bool Decompress(ConstBitView& source);
		protected:
			void Grow(const BitIndexType bitCount)
			{
				const BlockIndexType requiredCapacity = static_cast<BlockIndexType>(Math::Ceil((float)bitCount / (float)BitsPerBlock));
				if constexpr (AllocatorType::IsGrowable)
				{
					const BlockIndexType currentCapacity = m_allocator.GetCapacity();
					if (currentCapacity < requiredCapacity)
					{
						m_allocator.Allocate(requiredCapacity);
						m_allocator.GetRestrictedView().GetSubView(currentCapacity, requiredCapacity - currentCapacity).ZeroInitialize();
					}
				}
				else
				{
					Assert(m_allocator.GetCapacity() >= requiredCapacity);
				}
			}
		protected:
			template<typename OtherAllocatorType, uint64 OtherMaximumCount>
			friend struct BitsetBase;

			AllocatorType m_allocator;
		};
	}
}

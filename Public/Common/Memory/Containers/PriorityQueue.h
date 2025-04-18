#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Assert/Assert.h>
#include <Common/Memory/Forward.h>

#include <Common/Platform/NoInline.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/GetNumericSize.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Math/Clamp.h>
#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Threading/AtomicInteger.h>

#include <algorithm>

namespace ngine
{
	template<typename ContainedType, size_t Capacity, typename PriorityType>
	struct TPriorityQueue
	{
		using SizeType = Memory::NumericSize<Capacity>;
		static_assert(sizeof(SizeType) <= sizeof(uint32), "Size must be max half of uint64");
		using StorageSizeType = Memory::NumericSize<Math::NumericLimits<SizeType>::Max + 1>;
		static_assert(sizeof(StorageSizeType) == (sizeof(SizeType) * 2));

		using ElementView = ArrayView<ContainedType, SizeType>;
		using ConstElementView = ArrayView<const ContainedType, SizeType>;
		using PriorityView = ArrayView<PriorityType, SizeType>;
		using ConstPriorityView = ArrayView<const PriorityType, SizeType>;

		TPriorityQueue() = default;
		TPriorityQueue(const TPriorityQueue&) = delete;
		TPriorityQueue& operator=(const TPriorityQueue&) = delete;

		template<typename... Args>
		void Emplace(const PriorityType priority, Args&&... args) noexcept
		{
			while (true)
			{
				// Try to steal the second half of the available range
				Range rangeSecondHalf = (StorageSizeType)m_range;
				// The initial half of the range that we left in place
				const Range untouchedRange = Range{
					rangeSecondHalf.m_startIndex,
					Math::Clamp(SizeType(rangeSecondHalf.m_size >> (SizeType)1u), (SizeType)1u, rangeSecondHalf.m_size)
				};
				const bool exchanged = m_range.CompareExchangeWeak(rangeSecondHalf, untouchedRange);
				if (exchanged)
				{
					rangeSecondHalf.m_startIndex = untouchedRange.m_startIndex + untouchedRange.m_size;
					rangeSecondHalf.m_size -= untouchedRange.m_size;

					const PriorityView priorityView = m_priorities.GetSubView(rangeSecondHalf.m_startIndex, rangeSecondHalf.m_size);
					PriorityType* __restrict pPriorityEntry = std::upper_bound(
						priorityView.begin().Get(),
						priorityView.end().Get(),
						priority,
						[](const PriorityType priority, const PriorityType existingPriority) -> bool
						{
							return priority < existingPriority;
						}
					);

					auto emplaceElementIntoRange = [this, priority](
																					 Range& __restrict range,
																					 PriorityType* const __restrict pPriorityEntry,
																					 PriorityType* const __restrict pPriorityEnd,
																					 Args&&... args
																				 )
					{
						ContainedType* __restrict const pElement = &m_elements[m_priorities.GetIteratorIndex(pPriorityEntry)];
						const ContainedType* const pElementEnd = &m_elements[range.m_startIndex + range.m_size];

						if (pElement < pElementEnd)
						{
							// Move everything at upperBoundIt -> m_pEnd forward one unit
							ElementView(pElement + 1, pElementEnd + 1).CopyFromWithOverlap(ConstElementView(pElement, pElementEnd));
							PriorityView(pPriorityEntry + 1, pPriorityEnd + 1).CopyFromWithOverlap(ConstPriorityView(pPriorityEntry, pPriorityEnd));
						}

						*pElement = ContainedType(Forward<Args>(args)...);
						*pPriorityEntry = priority;

						range.m_size++;
					};

					if (pPriorityEntry == priorityView.begin())
					{
						// The priority might be placed into the initial half (untouchedRange), or in the middle between both
						// We can assume that nothing else was Emplaced at this point, since only one worker is allowed to Emplace
						// Hence items might be gone from the first half, but the order is the same

						// Before returning the second half, shift everything one unit to the right to make space for the new element
						{
							{
								ElementView elements = m_elements.GetSubView(rangeSecondHalf.m_startIndex, rangeSecondHalf.m_size);
								ElementView(elements.begin() + 1, elements.end() + 1).CopyFromWithOverlap(elements);
							}

							{
								PriorityView priorities = m_priorities.GetSubView(rangeSecondHalf.m_startIndex, rangeSecondHalf.m_size);
								PriorityView(priorities.begin() + 1, priorities.end() + 1).CopyFromWithOverlap(priorities);
							}

							rangeSecondHalf.m_startIndex++;
						}

						while (true)
						{
							// Swap the ranges by putting the second half of the available range back, and steal the current value
							Range rangeInitialHalf = (StorageSizeType)m_range;
							const bool exchangedRemainingRanges = m_range.CompareExchangeWeak(rangeInitialHalf, rangeSecondHalf);
							if (exchangedRemainingRanges)
							{
								const PriorityView newPriorityView = m_priorities.GetSubView(rangeInitialHalf.m_startIndex, rangeInitialHalf.m_size);
								pPriorityEntry = std::upper_bound(
									newPriorityView.begin().Get(),
									newPriorityView.end().Get(),
									priority,
									[](const PriorityType priority, const PriorityType existingPriority) -> bool
									{
										return priority < existingPriority;
									}
								);

								if (pPriorityEntry == newPriorityView.end())
								{
									// Place between this and the initial range
									*pPriorityEntry = priority;
									m_elements[m_priorities.GetIteratorIndex(pPriorityEntry)] = ContainedType(Forward<Args>(args)...);

									rangeInitialHalf.m_size++;
								}
								else
								{
									emplaceElementIntoRange(rangeInitialHalf, pPriorityEntry, newPriorityView.end(), Forward<Args>(args)...);

									// initial half: index 00000000, size 00000001
									// second half: index 00000001, size 00000001
									// m_range = rangeInitialHalf.m_storage;

									// index should be replaced with the initial half
									// size is always an add operation

									// TODO: We can change these two into one operation

									// Start by changing the initial index
									/*m_range.fetch_sub(Range((SizeType)rangeInitialHalf.m_size, (SizeType)0u).m_storage);
									// Then change the size, it's okay if processing starts on the index with the size off
									m_range.fetch_add(Range((SizeType)0u, rangeInitialHalf.m_size).m_storage);*/
								}

								while (true)
								{
									Range remainingRange = (StorageSizeType)m_range;
									const bool mergedRanges = m_range.CompareExchangeWeak(
										remainingRange,
										Range(rangeInitialHalf.m_startIndex, rangeInitialHalf.m_size + remainingRange.m_size)
									);
									if (mergedRanges)
									{
										break;
									}
								}

								break;
							}
						}
					}
					else
					{
						emplaceElementIntoRange(rangeSecondHalf, pPriorityEntry, priorityView.end(), Forward<Args>(args)...);

						// index should be unchanged (initial half start index is kept)
						// size is always an add operation
						m_range += Range((SizeType)0u, rangeSecondHalf.m_size);
					}

					return;
				}
			}
		}

		//! Shift the range back to index 0
		//! Requires moving all elements
		void ShiftToOrigin() noexcept
		{
			while (true)
			{
				// Try to steal the first half of the available range
				Range rangeFirstHalf = (StorageSizeType)m_range;
				if (rangeFirstHalf.m_startIndex == 0)
				{
					return;
				}

				if (rangeFirstHalf.m_size == 0)
				{
					const bool exchanged = m_range.CompareExchangeWeak(rangeFirstHalf, Range(0u, 0u));
					if (exchanged)
					{
						return;
					}
				}
				else
				{
					const SizeType maximumShiftCount = Math::Min(rangeFirstHalf.m_startIndex, SizeType(Capacity - rangeFirstHalf.m_size));
					const SizeType shiftedCount = Math::Min(SizeType(rangeFirstHalf.m_size >> (SizeType)1u), maximumShiftCount);
					const Range untouchedRange =
						Range{SizeType(rangeFirstHalf.m_startIndex + shiftedCount), SizeType(rangeFirstHalf.m_size - shiftedCount)};
					const bool exchanged = m_range.CompareExchangeWeak(rangeFirstHalf, untouchedRange);
					if (exchanged)
					{
						rangeFirstHalf.m_size -= untouchedRange.m_size;

						{
							ElementView elements = m_elements.GetSubView(rangeFirstHalf.m_startIndex, rangeFirstHalf.m_size);
							m_elements.GetSubView(rangeFirstHalf.m_startIndex - maximumShiftCount, rangeFirstHalf.m_size).CopyFromWithOverlap(elements);
						}

						{
							PriorityView priorities = m_priorities.GetSubView(rangeFirstHalf.m_startIndex, rangeFirstHalf.m_size);
							m_priorities.GetSubView(rangeFirstHalf.m_startIndex - maximumShiftCount, rangeFirstHalf.m_size)
								.CopyFromWithOverlap(priorities);
						}

						rangeFirstHalf.m_startIndex -= maximumShiftCount;

						// Swap
						while (true)
						{
							Range rangeSecondHalf = (StorageSizeType)m_range;
							const bool swappedRanges = m_range.CompareExchangeWeak(rangeSecondHalf, rangeFirstHalf);
							if (swappedRanges)
							{
								{
									ElementView elements = m_elements.GetSubView(rangeSecondHalf.m_startIndex, rangeSecondHalf.m_size);
									m_elements.GetSubView(rangeSecondHalf.m_startIndex - maximumShiftCount, rangeSecondHalf.m_size)
										.CopyFromWithOverlap(elements);
								}

								{
									PriorityView priorities = m_priorities.GetSubView(rangeSecondHalf.m_startIndex, rangeSecondHalf.m_size);
									m_priorities.GetSubView(rangeSecondHalf.m_startIndex - maximumShiftCount, rangeSecondHalf.m_size)
										.CopyFromWithOverlap(priorities);
								}

								// Merge rangeSecondHalf back into the main range
								m_range += Range((SizeType)0u, rangeSecondHalf.m_size);

								return;
							}
						}

						return;
					}
				}
			}
		}

		[[nodiscard]] bool HasElements() const noexcept
		{
			const Range availableRange = (StorageSizeType)m_range;
			return availableRange.m_size > 0;
		}

		[[nodiscard]] SizeType GetSize() const noexcept
		{
			const Range availableRange = (StorageSizeType)m_range;
			return availableRange.m_size;
		}

		[[nodiscard]] bool TryPop(ContainedType& element) noexcept
		{
			Range availableRange = (StorageSizeType)m_range;
			if (availableRange.m_size > 0)
			{
				[[maybe_unused]] ContainedType existingElement = m_elements[availableRange.m_startIndex];

				const bool exchanged =
					m_range.CompareExchangeStrong(availableRange, Range(availableRange.m_startIndex + 1u, availableRange.m_size - 1u));
				if (exchanged)
				{
					// Problem case: if owning thread inserts or overrides the element at this index during this iteration
					Assert(existingElement == m_elements[availableRange.m_startIndex]);
					element = m_elements[availableRange.m_startIndex];
					return true;
				}
			}

			return false;
		}

		[[nodiscard]] ElementView TryPopRange(SizeType count) noexcept
		{
			Range availableRange = (StorageSizeType)m_range;
			if (availableRange.m_size > 0)
			{
				[[maybe_unused]] ContainedType existingElement = m_elements[availableRange.m_startIndex];
				count = Math::Min(count, availableRange.m_size);

				const bool exchanged =
					m_range.CompareExchangeStrong(availableRange, Range(availableRange.m_startIndex + count, availableRange.m_size - count));
				if (exchanged)
				{
					// Problem case: if owning thread inserts or overrides the element at this index during this iteration
					Assert(existingElement == m_elements[availableRange.m_startIndex]);
					return ElementView{&m_elements[availableRange.m_startIndex], &m_elements[availableRange.m_startIndex + count]};
				}
			}

			return ElementView();
		}
	protected:
		Array<ContainedType, Capacity> m_elements;
		Array<PriorityType, Capacity> m_priorities;

		struct Range
		{
			constexpr Range(const SizeType startIndex, const SizeType size) noexcept
				: m_startIndex(startIndex)
				, m_size(size)
			{
			}

			constexpr Range(const StorageSizeType storage) noexcept
				: m_storage{storage}
			{
			}

			[[nodiscard]] constexpr FORCE_INLINE operator StorageSizeType() const noexcept
			{
				return m_storage;
			}
			[[nodiscard]] constexpr FORCE_INLINE operator StorageSizeType&() noexcept
			{
				return m_storage;
			}

			union
			{
				struct
				{
					SizeType m_startIndex;
					SizeType m_size;
				};
				StorageSizeType m_storage;
			};
		};

		// Range of m_allocator that is currently queued
		// Stored as one atomic in order to allow for one atomic operation where we change the entire range
		Threading::Atomic<StorageSizeType> m_range = 0u;
	};
}

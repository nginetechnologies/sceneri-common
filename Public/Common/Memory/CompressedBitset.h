#pragma once

#include <Common/Memory/Containers/InlineVector.h>
#include <Common/Memory/Bitset.h>
#include <Common/Memory/UniquePtr.h>
#include <Common/Memory/New.h>
#include <Common/Math/Range.h>

#include <algorithm>

namespace ngine
{
	template<uint64 MaximumCount>
	struct CompressedBitset
	{
		inline static constexpr uint16 MaximumBitsPerContainer = 65535;
		inline static constexpr uint16 ContainerCount = (uint16)Math::Ceil((float)MaximumCount / (float)MaximumBitsPerContainer);
		using BitIndexType = Memory::NumericSize<MaximumCount>;
		using ContainerIndexType = Memory::NumericSize<ContainerCount>;
		using ContainerBitIndex = uint16;

		CompressedBitset() = default;
		CompressedBitset(Memory::SetAllType)
		{
			SetAll();
		}
		~CompressedBitset()
		{
			for (ContainerIndexType containerIndex = 0; containerIndex < ContainerCount; containerIndex++)
			{
				Container& container = m_containers[containerIndex];
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						break;
					case ContainerType::Indices:
						container.m_indices.~Indices();
						break;
					case ContainerType::Bitset:
						Assert(container.m_pBitset.IsValid());
						container.m_pBitset.DestroyElement();
						break;
					case ContainerType::Ranges:
						container.m_ranges.~Ranges();
						break;
				}
			}
		}
	protected:
		enum class ContainerType : uint8
		{
			None,
			//! Contains a vector of indices
			Indices,
			//! Bitset containing 65536 bitsets
			Bitset,
			//! Range of fully set bits
			Ranges
		};

		using Indices = InlineVector<uint16, 4, uint16>;
		using Range = Math::Range<uint16>;
		using Ranges = InlineVector<Range, 2, uint16>;

		//! Represents a container holding integers of a 16-bit range (0 - 65535)
		struct Container
		{
			Container()
			{
			}
			~Container()
			{
			}

			union
			{
				//! Vector containing individual indices of elements
				Indices m_indices;
				//! Vector containing a range of set bits
				Ranges m_ranges;
				UniquePtr<Bitset<MaximumBitsPerContainer>> m_pBitset;
			};
		};
	public:
		[[nodiscard]] PURE_STATICS bool IsSet(const BitIndexType position) const
		{
			const ContainerIndexType containerIndex = GetContainerIndex(position);
			const Container& container = m_containers[containerIndex];
			switch (m_containerTypes[containerIndex])
			{
				case ContainerType::None:
					return false;
				case ContainerType::Indices:
					return std::binary_search(
						&*container.m_indices.begin(),
						&*container.m_indices.end(),
						GetContainerBitIndex(position),
						[](const ContainerBitIndex left, const ContainerBitIndex right)
						{
							return left < right;
						}
					);
				case ContainerType::Bitset:
					return container.m_pBitset->IsSet(GetContainerBitIndex(position));
				case ContainerType::Ranges:
				{
					const ContainerBitIndex containerBitIndex = GetContainerBitIndex(position);
					auto it = std::lower_bound(
						&*container.m_ranges.begin(),
						&*container.m_ranges.end(),
						containerBitIndex,
						[](const Range range, ContainerBitIndex containerBitIndex)
						{
							return range.GetEnd() <= containerBitIndex;
						}
					);

					// Check if `value` is within the found run
					if (it != container.m_ranges.end())
					{
						return it->Contains(containerBitIndex);
					}
					return false;
				}
			}
			ExpectUnreachable();
		}
		[[nodiscard]] PURE_STATICS bool IsNotSet(const BitIndexType position) const
		{
			return !IsSet(position);
		}

		[[nodiscard]] PURE_STATICS bool AreNoneSet() const
		{
			for (ContainerIndexType containerIndex = 0; containerIndex < ContainerCount; ++containerIndex)
			{
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						break;
					case ContainerType::Indices:
					{
						if (m_containers[containerIndex].m_indices.HasElements())
						{
							return false;
						}
					}
					break;
					case ContainerType::Bitset:
					{
						if (m_containers[containerIndex].m_pBitset->AreAnySet())
						{
							return false;
						}
					}
					break;
					case ContainerType::Ranges:
					{
						if (m_containers[containerIndex].m_ranges.HasElements())
						{
							return false;
						}
					}
					break;
				}
			}
			return true;
		}
		/*[[nodiscard]] PURE_STATICS bool AreNoneSet(const CompressedBitset& other) const
		{
		}*/
		[[nodiscard]] PURE_STATICS bool AreAnySet() const
		{
			return !AreNoneSet();
		}
		/*[[nodiscard]] PURE_STATICS bool AreAnySet(const CompressedBitset& other) const
		{
		}*/
		[[nodiscard]] PURE_STATICS bool AreAllSet() const
		{
			for (ContainerIndexType containerIndex = 0; containerIndex < (ContainerCount - 1); ++containerIndex)
			{
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						return false;
					case ContainerType::Indices:
					{
						if (m_containers[containerIndex].m_indices.GetSize() < MaximumBitsPerContainer)
						{
							return false;
						}
					}
					break;
					case ContainerType::Bitset:
					{
						if (!m_containers[containerIndex].m_pBitset->AreAllSet())
						{
							return false;
						}
					}
					break;
					case ContainerType::Ranges:
					{
						if (m_containers[containerIndex].m_ranges.GetSize() != 1 || m_containers[containerIndex].m_ranges[0].GetSize() < MaximumBitsPerContainer)
						{
							return true;
						}
					}
					break;
				}
			}
			return GetNumberOfSetBits(ContainerCount - 1) == (MaximumCount % MaximumBitsPerContainer);
		}
		[[nodiscard]] PURE_STATICS bool AreAnyNotSet() const
		{
			return !AreAllSet();
		}
		/*[[nodiscard]] PURE_STATICS bool AreAllSet(const CompressedBitset& other) const
		{
		}
		[[nodiscard]] PURE_STATICS bool AreAnyNotSet(const CompressedBitset& other) const
		{
		}
		PURE_STATICS CompressedBitset operator&(const CompressedBitset& __restrict other) const
		{
		}
		CompressedBitset& operator&=(const CompressedBitset& __restrict other)
		{
		}
		PURE_STATICS CompressedBitset operator|(const CompressedBitset& __restrict other) const
		{
		}
		CompressedBitset& operator|=(const CompressedBitset& __restrict other)
		{
		}
		PURE_STATICS CompressedBitset operator^(const CompressedBitset& __restrict other) const
		{
		}
		CompressedBitset& operator^=(const CompressedBitset& __restrict other)
		{
		}
		CompressedBitset operator~() const noexcept
		{
		}*/
		bool operator==(const CompressedBitset& __restrict other) const
		{
			for (ContainerIndexType containerIndex = 0; containerIndex < ContainerCount; ++containerIndex)
			{
				if (m_containerTypes[containerIndex] != other.m_containerTypes[containerIndex])
				{
					return false;
				}

				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						break;
					case ContainerType::Indices:
					{
						if (m_containers[containerIndex].m_indices.GetView() != other.m_containers[containerIndex].m_indices.GetView())
						{
							return false;
						}
					}
					break;
					case ContainerType::Bitset:
					{
						if (*m_containers[containerIndex].m_pBitset != *other.m_containers[containerIndex].m_pBitset)
						{
							return false;
						}
					}
					break;
					case ContainerType::Ranges:
					{
						if (m_containers[containerIndex].m_ranges.GetView() != other.m_containers[containerIndex].m_ranges.GetView())
						{
							return false;
						}
					}
					break;
				}
			}
			return true;
		}
		bool operator!=(const CompressedBitset& __restrict other) const
		{
			return !operator==(other);
		}

		void Set(const BitIndexType position)
		{
			const ContainerIndexType containerIndex = GetContainerIndex(position);
			const ContainerBitIndex containerBitIndex = GetContainerBitIndex(position);
			Container& container = m_containers[containerIndex];
			switch (m_containerTypes[containerIndex])
			{
				case ContainerType::None:
				{
					m_containerTypes[containerIndex] = ContainerType::Ranges;
					new (&container.m_ranges) Ranges();

					container.m_ranges.EmplaceBack(Range::Make(containerBitIndex, 1));
				}
				break;
				case ContainerType::Indices:
				{
					// TODO: Logic to switch containers
					if (container.m_indices.EmplaceBackUnique(ContainerBitIndex{containerBitIndex}))
					{
						std::sort(
							&*container.m_indices.begin(),
							&*container.m_indices.end(),
							[](const ContainerBitIndex left, const ContainerBitIndex right)
							{
								return left < right;
							}
						);
					}
				}
				break;
				case ContainerType::Bitset:
					container.m_pBitset->Set(GetContainerBitIndex(position));
					break;
				case ContainerType::Ranges:
				{
					auto it = std::lower_bound(
						&*container.m_ranges.begin(),
						&*container.m_ranges.end(),
						containerBitIndex,
						[](const Range range, ContainerBitIndex containerBitIndex)
						{
							return range.GetEnd() <= containerBitIndex;
						}
					);
					// Check if a range already contains this value
					if (it != container.m_ranges.end() && it->Contains(containerBitIndex))
					{
						return;
					}

					if (it != container.m_ranges.begin() && (it - 1)->GetEnd() == containerBitIndex)
					{
						// Append
						--it;
						Assert(it->GetMaximum() <= MaximumBitsPerContainer);
						*it = Range::Make(it->GetMinimum(), it->GetSize() + 1);

						// Merge with the next range if they are now contiguous
						const auto nextIt = it + 1;
						if (nextIt != container.m_ranges.end() && it->GetEnd() == nextIt->GetMinimum())
						{
							*it = Range::Make(it->GetMinimum(), it->GetSize() + nextIt->GetSize());
							container.m_ranges.Remove(nextIt); // Erase the next range since it's merged
						}
					}
					else if (it != container.m_ranges.end() && it->GetMinimum() == containerBitIndex + 1)
					{
						// Prepend
						Assert(it->GetMinimum() > 0);
						*it = Range::Make(it->GetMinimum() - 1, it->GetSize() + 1);

						// Merge with the previous range if they are now contiguous
						auto previousIt = it - 1;
						if (it != container.m_ranges.begin() && previousIt->GetMaximum() == it->GetMinimum())
						{
							--it; // Move iterator to the previous range
							*it = Range::Make(it->GetMinimum(), it->GetSize() + (it + 1)->GetSize());
							container.m_ranges.Remove(it + 1); // Erase the current range since it's merged
						}
					}
					else
					{
						// Insert new range
						container.m_ranges.Emplace(it, Memory::Uninitialized, Range::Make(containerBitIndex, 1));
					}
				}
				break;
			}
		}

		void Clear(const BitIndexType position)
		{
			const ContainerIndexType containerIndex = GetContainerIndex(position);
			const ContainerBitIndex containerBitIndex = GetContainerBitIndex(position);
			Container& container = m_containers[containerIndex];
			switch (m_containerTypes[containerIndex])
			{
				case ContainerType::None:
					break;
				case ContainerType::Indices:
				{
					// TODO: Logic to switch containers
					const auto it = std::lower_bound(&*container.m_indices.begin(), &*container.m_indices.end(), containerBitIndex);
					if (it != container.m_indices.end() && *it == containerBitIndex)
					{
						container.m_indices.Remove(it);
					}
				}
				break;
				case ContainerType::Bitset:
					// TODO: Logic for switching containers
					container.m_pBitset->Clear(GetContainerBitIndex(position));
					break;
				case ContainerType::Ranges:
				{
					auto it = std::lower_bound(
						&*container.m_ranges.begin(),
						&*container.m_ranges.end(),
						containerBitIndex,
						[](const Range range, ContainerBitIndex containerBitIndex)
						{
							return range.GetEnd() <= containerBitIndex;
						}
					);
					if (it != container.m_ranges.end())
					{
						if (it->Contains(containerBitIndex))
						{
							if (it->GetSize() == 1)
							{
								// Remove the whole range
								container.m_ranges.Remove(it);
							}
							else if (containerBitIndex == it->GetMinimum())
							{
								++(*it);
							}
							else if (containerBitIndex == it->GetMaximum() - 1)
							{
								--(*it);
							}
							else
							{
								// Value is in the middle of the run, split into two runs
								const ContainerBitIndex newStart = containerBitIndex + 1;
								const ContainerBitIndex newLength = it->GetEnd() - newStart;
								*it = Range::Make(it->GetMinimum(), containerBitIndex - it->GetMinimum());
								container.m_ranges.Emplace(it + 1, Memory::Uninitialized, Range::Make(newStart, newLength));
							}
							return;
						}
					}
				}
				break;
			}
		}

		// TODO: Iterator

		void SetAll()
		{
			for (ContainerIndexType containerIndex = 0; containerIndex < (ContainerCount - 1); ++containerIndex)
			{
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						new (&m_containers[containerIndex].m_ranges) Ranges();
						m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, MaximumBitsPerContainer));
						break;
					case ContainerType::Indices:
						m_containers[containerIndex].m_indices.~Indices();
						new (&m_containers[containerIndex].m_ranges) Ranges();
						m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, MaximumBitsPerContainer));
						break;
					case ContainerType::Bitset:
						m_containers[containerIndex].m_pBitset.DestroyElement();
						new (&m_containers[containerIndex].m_ranges) Ranges();
						m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, MaximumBitsPerContainer));
						break;
					case ContainerType::Ranges:
						m_containers[containerIndex].m_ranges.Clear();
						m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, MaximumBitsPerContainer));
						break;
				}
			}
			ContainerIndexType containerIndex = ContainerCount - 1;
			const ContainerBitIndex lastContainerBitCount = (ContainerBitIndex)(MaximumCount % MaximumBitsPerContainer);
			switch (m_containerTypes[containerIndex])
			{
				case ContainerType::None:
					new (&m_containers[containerIndex].m_ranges) Ranges();
					m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, lastContainerBitCount));
					break;
				case ContainerType::Indices:
					m_containers[containerIndex].m_indices.~Indices();
					new (&m_containers[containerIndex].m_ranges) Ranges();
					m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, lastContainerBitCount));
					break;
				case ContainerType::Bitset:
					m_containers[containerIndex].m_pBitset.DestroyElement();
					new (&m_containers[containerIndex].m_ranges) Ranges();
					m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, lastContainerBitCount));
					break;
				case ContainerType::Ranges:
					m_containers[containerIndex].m_ranges.Clear();
					m_containers[containerIndex].m_ranges.EmplaceBack(Range::Make(0, lastContainerBitCount));
					break;
			}
			m_containerTypes.GetView().InitializeAll(ContainerType::Ranges);
		}
		void ClearAll()
		{
			for (ContainerIndexType containerIndex = 0; containerIndex < ContainerCount; ++containerIndex)
			{
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						break;
					case ContainerType::Indices:
						m_containers[containerIndex].m_indices.~Indices();
						break;
					case ContainerType::Bitset:
						m_containers[containerIndex].m_pBitset.DestroyElement();
						break;
					case ContainerType::Ranges:
						m_containers[containerIndex].m_ranges.~Ranges();
						break;
				}
			}
			m_containerTypes.GetView().ZeroInitialize();
		}

		[[nodiscard]] inline PURE_STATICS BitIndexType GetNumberOfSetBits() const
		{
			BitIndexType count{0};
			for (ContainerIndexType containerIndex = 0; containerIndex < ContainerCount; ++containerIndex)
			{
				count += GetNumberOfSetBits(containerIndex);
			}
			return count;
		}

		[[nodiscard]] PURE_STATICS Memory::BitIndex<BitIndexType> GetFirstSetIndex() const
		{
			BitIndexType offset{0};
			for (ContainerIndexType containerIndex = 0; containerIndex < ContainerCount; ++containerIndex)
			{
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						break;
					case ContainerType::Indices:
						return {(BitIndexType)(offset + m_containers[containerIndex].m_indices[0] + 1)};
					case ContainerType::Bitset:
						return {(BitIndexType)(offset + *m_containers[containerIndex].m_pBitset->GetFirstSetIndex() + 1)};
					case ContainerType::Ranges:
						return {(BitIndexType)(offset + m_containers[containerIndex].m_ranges[0].GetMinimum() + 1)};
				}
				offset += MaximumBitsPerContainer;
			}
			return {};
		}

		[[nodiscard]] PURE_STATICS Memory::BitIndex<BitIndexType> GetLastSetIndex() const
		{
			BitIndexType offset{MaximumBitsPerContainer * (ContainerCount - 1)};
			for (auto it = m_containers.end() - 1, endIt = m_containers.begin() - 1; it > endIt; --it)
			{
				const ContainerIndexType containerIndex = m_containers.GetIteratorIndex(it);
				switch (m_containerTypes[containerIndex])
				{
					case ContainerType::None:
						break;
					case ContainerType::Indices:
						return {(BitIndexType)(offset + m_containers[containerIndex].m_indices.GetLastElement() + 1)};
					case ContainerType::Bitset:
						return {(BitIndexType)(offset + *m_containers[containerIndex].m_pBitset->GetLastSetIndex() + 1)};
					case ContainerType::Ranges:
						return {(BitIndexType)(offset + m_containers[containerIndex].m_ranges.GetLastElement().GetMaximum() + 1)};
				}
				offset -= MaximumBitsPerContainer;
			}
			return {};
		}
	protected:
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr static ContainerIndexType GetContainerIndex(const BitIndexType bitIndex)
		{
			return static_cast<ContainerIndexType>(bitIndex / MaximumBitsPerContainer);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr static ContainerBitIndex GetContainerBitIndex(const BitIndexType bitIndex)
		{
			return static_cast<ContainerBitIndex>(bitIndex % MaximumBitsPerContainer);
		}

		/*void ConvertRangesToIndices(const ContainerIndexType containerIndex)
		{
		  Container& container = m_containers[containerIndex];
		  Indices newIndices;

		}*/

		[[nodiscard]] inline PURE_STATICS uint16 GetNumberOfSetBits(const ContainerIndexType containerIndex) const
		{
			switch (m_containerTypes[containerIndex])
			{
				case ContainerType::None:
					return 0;
				case ContainerType::Indices:
					return m_containers[containerIndex].m_indices.GetSize();
				case ContainerType::Bitset:
					return m_containers[containerIndex].m_pBitset->GetNumberOfSetBits();
				case ContainerType::Ranges:
				{
					uint16 count{0};
					for (const Range range : m_containers[containerIndex].m_ranges)
					{
						count += range.GetSize();
					}
					return count;
				}
			}
			ExpectUnreachable();
		}
	protected:
		Array<Container, ContainerCount> m_containers;
		Array<ContainerType, ContainerCount> m_containerTypes{Memory::Zeroed};
	};
}

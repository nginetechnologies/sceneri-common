#pragma once

#include <Common/Assert/Assert.h>
#include <Common/TypeTraits/TypeConstant.h>
#include <Common/TypeTraits/Void.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include <Common/TypeTraits/IsCopyAssignable.h>
#include <Common/TypeTraits/IsMoveAssignable.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/Max.h>
#include <Common/Math/PowerOfTwo.h>
#include <Common/Math/Vectorization/PackedInt8.h>
#include <Common/Memory/Allocators/Allocate.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Memory/Copy.h>
#include <Common/Memory/Set.h>
#include <Common/Memory/Swap.h>
#include <Common/Memory/CountBits.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/ForceInline.h>

namespace ngine
{
	namespace Internal
	{
		template<typename, typename = void>
		struct TIsTransparent : TypeTraits::FalseType
		{
		};
		template<typename T>
		struct TIsTransparent<T, TypeTraits::Void<typename T::is_transparent>> : TypeTraits::TrueType
		{
		};
		template<typename T>
		inline static constexpr bool IsTransparent = TIsTransparent<T>::Value;
	}

	template<typename Key, typename KeyValue, typename HashTableDetail, typename HashType, typename KeyEqual, typename AllocatorType_>
	struct THashTable
	{
		/// Properties
		using ValueType = KeyValue;
		using SizeType = uint32;
		using AllocatorType = AllocatorType_;
	private:
		using KeyValueView = ArrayView<KeyValue, SizeType>;
		using ConstKeyValueView = ArrayView<const KeyValue, SizeType>;
		using ControlMask = Math::Vectorization::Packed<uint8, 16>;
		using ControlView = ArrayView<uint8, SizeType>;
		using ConstControlView = ArrayView<const uint8, SizeType>;

		template<typename TableType, typename Iterator>
		struct IteratorBase
		{
			IteratorBase() = delete;
			IteratorBase(const IteratorBase& other) = default;
			IteratorBase& operator=(const IteratorBase& other) = default;
			IteratorBase(IteratorBase&& other) = default;
			IteratorBase& operator=(IteratorBase&& other) = default;

			explicit constexpr IteratorBase(TableType& table)
				: m_table(table)
				, m_index(0)
			{
				SizeType index{0};
				const SizeType bucketCount{m_table->GetBucketCount()};
				const ConstControlView controlBytes = m_table->GetControlBytes();
				while (index < bucketCount && (controlBytes[index] & UsedBucketMask) == 0)
				{
					++index;
				}
				m_index = index;
			}

			constexpr IteratorBase(TableType& table, const SizeType index)
				: m_table(table)
				, m_index(index)
			{
			}

			constexpr Iterator operator++()
			{
				Assert(IsValid());

				SizeType index{m_index};
				const SizeType bucketCount{m_table->GetBucketCount()};
				const ConstControlView controlBytes = m_table->GetControlBytes();
				do
				{
					++index;
				} while (index < bucketCount && (controlBytes[index] & UsedBucketMask) == 0);
				m_index = index;

				return static_cast<Iterator&>(*this);
			}
			constexpr Iterator operator++(int)
			{
				Iterator result(m_table, m_index);
				++(*this);
				return result;
			}

			[[nodiscard]] constexpr const KeyValue& operator*() const
			{
				Assert(IsValid());
				return m_table->GetKeyValues()[m_index];
			}
			[[nodiscard]] constexpr const KeyValue* operator->() const
			{
				Assert(IsValid());
				return Memory::GetAddressOf(m_table->GetKeyValues()[m_index]);
			}

			[[nodiscard]] constexpr bool operator==(const Iterator& other) const
			{
				return m_index == other.m_index && &m_table == &other.m_table;
			}

			[[nodiscard]] constexpr bool operator!=(const Iterator& other) const
			{
				return !(*this == other);
			}

			[[nodiscard]] constexpr bool IsValid() const
			{
				return m_index < m_table->GetBucketCount() && (m_table->GetControlBytes()[m_index] & UsedBucketMask) != 0;
			}
		protected:
			friend THashTable;

			ReferenceWrapper<TableType> m_table;
			SizeType m_index;
		};

		/// Get the maximum number of elements that we can support given a number of buckets
		[[nodiscard]] static constexpr SizeType GetMaximumElementCount(const SizeType bucketCount)
		{
			return SizeType((MaximumLoadFactorNumerator * bucketCount) / MaximumLoadFactorDenominator);
		}

		FORCE_INLINE void SetBucketControlValue(const SizeType bucketIndex, const uint8 value)
		{
			const SizeType bucketCount{GetBucketCount()};
			Assert(bucketIndex < bucketCount);
			const ControlView controlBytes = GetControlBytes();
			controlBytes[bucketIndex] = value;
			controlBytes[((bucketIndex - 15) & (bucketCount - 1)) + 15] = value;
		}

		FORCE_INLINE void GetBucketIndexAndControlValue(const uint64 keyHashValue, SizeType& bucketIndexOut, uint8& controlOut) const
		{
			// Split hash into index and control value
			const SizeType bucketCount = GetBucketCount();
			bucketIndexOut = SizeType(keyHashValue >> 7) & (bucketCount - 1);
			controlOut = UsedBucketMask | uint8(keyHashValue);
		}

		void AllocateTable(const SizeType bucketCapacity)
		{
			Assert(m_allocator.GetData() == nullptr);

			m_remainingCapacity = GetMaximumElementCount(bucketCapacity);
			m_allocator.Allocate(uint32(size(bucketCapacity) * (sizeof(KeyValue) + 1) + 15));
			Assert(GetBucketCount() == bucketCapacity);
		}

		void CopyTable(const THashTable& other)
		{
			if (other.HasElements())
			{
				AllocateTable(other.GetBucketCount());

				// Copy control bytes
				const ControlView controlBytes = GetControlBytes();
				controlBytes.CopyFrom(other.GetControlBytes());

				const KeyValueView keyValues = GetKeyValues();
				const ConstKeyValueView otherKeyValues = other.GetKeyValues();
				// Copy elements
				SizeType index{0};
				for (ByteType *controlIt = controlBytes.begin(), *endIt = controlIt + GetBucketCount(); controlIt != endIt; ++controlIt, ++index)
				{
					if (*controlIt & UsedBucketMask)
					{
						new (Memory::GetAddressOf(keyValues[index])) KeyValue(otherKeyValues[index]);
					}
				}
				m_size = other.m_size;
			}
		}

		void GrowTable(const SizeType newMaximumSize)
		{
			// Calculate new size
			const SizeType previousBucketCount{GetBucketCount()};

			const KeyValueView previousKeyValues = GetKeyValues();
			const ConstControlView previousControlBytes = GetControlBytes();
			AllocatorType previousAllocator = Move(m_allocator);
			m_size = 0;
			m_remainingCapacity = 0;

			AllocateTable(newMaximumSize);

			// Reset all control bytes
			const ControlView controlBytes = GetControlBytes();
			if (controlBytes.HasElements())
			{
				controlBytes.ZeroInitialize();
			}

			const KeyValueView newKeyValues = GetKeyValues();
			if (previousKeyValues.HasElements())
			{
				// Copy all elements from the old table
				for (SizeType i = 0; i < previousBucketCount; ++i)
				{
					if (previousControlBytes[i] & UsedBucketMask)
					{
						SizeType index;
						KeyValue& element = previousKeyValues[i];
						const Key& key = HashTableDetail::GetKey(element);
						const uint64 keyHashValue = HashType{}(key);
						[[maybe_unused]] const bool wasInserted = InsertKey<true>(key, keyHashValue, index);
						Assert(wasInserted);
						new (Memory::GetAddressOf(newKeyValues[index])) KeyValue(Move(element));
						if constexpr (!TypeTraits::IsTriviallyDestructible<KeyValue>)
						{
							element.~KeyValue();
						}
					}
				}

				previousAllocator.Free();
			}
		}
	protected:
		template<bool InsertAfterGrow = false>
		bool InsertKey(const Key& key, const uint64 keyHashValue, SizeType& indexOut)
		{
			// Ensure we have enough space
			if (m_remainingCapacity == 0)
			{
				// Should not be growing if we're already growing!
				if constexpr (!InsertAfterGrow)
				{
					const SizeType bucketCount = GetBucketCount();
					// Decide if we need to clean up all tombstones or if we need to grow the map
					const SizeType deletedCount = GetMaximumElementCount(bucketCount) - m_size;
					if (deletedCount * MaximumDeletedElementsDenominator > bucketCount * MaximumDeletedElementsNumerator)
					{
						Rehash();
					}
					else
					{
						// Resize to the next power of two
						const SizeType newMaximumSize = Math::Max(SizeType(m_allocator.GetCapacity() << 1), (SizeType)16);
						Assert(newMaximumSize >= m_allocator.GetCapacity(), "Hash table overflow!");
						GrowTable(newMaximumSize);
					}
				}
				else
				{
					ExpectUnreachable();
				}
			}

			// Split hash into index and control value
			SizeType index;
			uint8 control;
			GetBucketIndexAndControlValue(keyHashValue, index, control);

			// Keeps track of the index of the first deleted bucket we found
			constexpr SizeType NoDeletedMask = ~SizeType(0);
			SizeType firstDeletedIndex = NoDeletedMask;

			const SizeType bucketCount = GetBucketCount();

			// Linear probing
			KeyEqual equal;
			const SizeType bucketMask = bucketCount - 1;
			const ControlMask control16{control};
			const ControlMask bucketEmpty{Math::Zero};
			const ControlMask bucketDeleted{DeletedBucketMask};
			const ControlView allControlBytes = GetControlBytes();
			const KeyValueView keyValues = GetKeyValues();
			for (;;)
			{
				// Read 16 control values (note that we added 15 bytes at the end of the control values that mirror the first 15 bytes)
				Assert(index + 16 <= allControlBytes.GetSize());
				const ControlMask controlBytes{Math::Vectorization::LoadUnaligned, &allControlBytes[index]};

				// Check if we must find the element before we can Insert
				if constexpr (!InsertAfterGrow)
				{
					// Check for the control value we're looking for
					// Note that when deleting we can create empty buckets instead of deleted buckets.
					// This means we must unconditionally check all buckets in this batch for equality
					// (also beyond the first empty bucket).
					uint32 controlEqual = uint32((controlBytes == control16).GetMask());

					// Index within the 16 buckets
					SizeType localIndex = index;

					// Loop while there's still buckets to process
					while (controlEqual != 0)
					{
						// Get the first equal bucket
						const uint32 firstEqual = Memory::GetNumberOfTrailingZeros(controlEqual);

						// Skip to the bucket
						localIndex += firstEqual;

						// Make sure that our index is not beyond the end of the table
						localIndex &= bucketMask;

						// We found a bucket with same control value
						if (equal(HashTableDetail::GetKey(keyValues[localIndex]), key))
						{
							// Element already exists
							indexOut = localIndex;
							return false;
						}

						// Skip past this bucket
						controlEqual >>= firstEqual + 1;
						localIndex++;
					}

					// Check if we're still scanning for deleted buckets
					if (firstDeletedIndex == NoDeletedMask)
					{
						// Check if any buckets have been deleted, if so store the first one
						const uint32 controlDeleted = uint32((controlBytes == bucketDeleted).GetMask());
						if (controlDeleted != 0)
							firstDeletedIndex = index + Memory::GetNumberOfTrailingZeros(controlDeleted);
					}
				}

				// Check for empty buckets
				uint32 controlEmpty = uint32((controlBytes == bucketEmpty).GetMask());
				if (controlEmpty != 0)
				{
					// If we found a deleted bucket, use it.
					// It doesn't matter if it is before or after the first empty bucket we found
					// since we will always be scanning in batches of 16 buckets.
					if (firstDeletedIndex == NoDeletedMask || InsertAfterGrow)
					{
						index += Memory::GetNumberOfTrailingZeros(controlEmpty);
						--m_remainingCapacity; // Using an empty bucket decreases the load left
					}
					else
					{
						index = firstDeletedIndex;
					}

					// Make sure that our index is not beyond the end of the table
					index &= bucketMask;

					// Update control byte
					SetBucketControlValue(index, control);
					++m_size;

					// Return index to newly allocated bucket
					indexOut = index;
					return true;
				}

				// Move to next batch of 16 buckets
				index = (index + 16) & bucketMask;
			}
		}
	public:
		struct const_iterator;

		/// Non-const iterator
		struct iterator : public IteratorBase<THashTable, iterator>
		{
			using BaseType = IteratorBase<THashTable, iterator>;

			explicit iterator(THashTable& table)
				: BaseType(table)
			{
			}
			iterator(THashTable& table, const SizeType index)
				: BaseType(table, index)
			{
			}
			iterator(const iterator& iterator)
				: BaseType(iterator)
			{
			}

			iterator& operator=(const iterator& other)
			{
				BaseType::operator=(other);
				return *this;
			}

			using BaseType::operator*;
			[[nodiscard]] KeyValue& operator*() const
			{
				Assert(BaseType::IsValid());
				return BaseType::m_table->GetKeyValues()[BaseType::m_index];
			}

			using BaseType::operator->;
			[[nodiscard]] KeyValue* operator->() const
			{
				Assert(BaseType::IsValid());
				return Memory::GetAddressOf(BaseType::m_table->GetKeyValues()[BaseType::m_index]);
			}

			friend const_iterator;
			friend IteratorBase<const THashTable, const_iterator>;
		};

		/// Const iterator
		struct const_iterator : public IteratorBase<const THashTable, const_iterator>
		{
			using BaseType = IteratorBase<const THashTable, const_iterator>;

			explicit const_iterator(const THashTable& table)
				: BaseType(table)
			{
			}
			const_iterator(const THashTable& table, const SizeType index)
				: BaseType(table, index)
			{
			}
			const_iterator(const const_iterator& other)
				: BaseType(other)
			{
			}
			const_iterator(const iterator& iterator)
				: BaseType(iterator.BaseType::m_table, iterator.BaseType::m_index)
			{
			}

			const_iterator& operator=(const iterator& other)
			{
				BaseType::m_table = other.m_table;
				BaseType::m_index = other.m_index;
				return *this;
			}
			const_iterator& operator=(const const_iterator& other)
			{
				BaseType::operator=(other);
				return *this;
			}
		};

		THashTable() = default;
		THashTable(Memory::ReserveType, const SizeType size)
		{
			Reserve(size);
		}
		THashTable(const THashTable& other)
		{
			CopyTable(other);
		}
		THashTable(THashTable&& other) noexcept
			: m_allocator(Move(other.m_allocator))
			, m_size(other.m_size)
			, m_remainingCapacity(other.m_remainingCapacity)
		{
			other.m_size = 0;
			other.m_remainingCapacity = 0;
		}

		THashTable& operator=(const THashTable& other) LIFETIME_BOUND
		{
			Assert(this != &other);
			Clear();
			CopyTable(other);
			return *this;
		}
		THashTable& operator=(THashTable&& other) noexcept LIFETIME_BOUND
		{
			Assert(this != &other);
			Clear();

			m_allocator = Move(other.m_allocator);
			m_size = other.m_size;
			m_remainingCapacity = other.m_remainingCapacity;

			other.m_size = 0;
			other.m_remainingCapacity = 0;
			return *this;
		}
		~THashTable()
		{
			Clear();
		}

		void Reserve(const SizeType requestedMaximumSize)
		{
			// Calculate max size based on load factor
			const SizeType maximumSize = Math::NearestPowerOfTwo(
				Math::Max(SizeType((MaximumLoadFactorDenominator * requestedMaximumSize) / MaximumLoadFactorNumerator), 16u)
			);
			if (maximumSize > GetBucketCount())
			{
				GrowTable(maximumSize);
			}
		}

		void Clear()
		{
			// Delete all elements
			if constexpr (!TypeTraits::IsTriviallyDestructible<KeyValue>)
			{
				if (!IsEmpty())
				{
					const ConstControlView controlBytes = GetControlBytes();
					const ConstKeyValueView keyValues = GetKeyValues();
					const SizeType bucketCount = GetBucketCount();
					for (SizeType i = 0; i < bucketCount; ++i)
					{
						if (controlBytes[i] & UsedBucketMask)
						{
							keyValues[i].~KeyValue();
						}
					}
				}
			}

			m_size = 0;
			const ControlView controlBytes = GetControlBytes();
			if (controlBytes.HasElements())
			{
				controlBytes.ZeroInitialize();
			}
			m_remainingCapacity = GetMaximumElementCount(GetBucketCount());
		}

		/// Iterator to first element
		[[nodiscard]] iterator begin() LIFETIME_BOUND
		{
			return iterator(*this);
		}
		[[nodiscard]] iterator end() LIFETIME_BOUND
		{
			return iterator(*this, GetBucketCount());
		}

		[[nodiscard]] const_iterator begin() const LIFETIME_BOUND
		{
			return const_iterator(*this);
		}
		[[nodiscard]] const_iterator cbegin() const LIFETIME_BOUND
		{
			return const_iterator(*this);
		}

		[[nodiscard]] const_iterator end() const LIFETIME_BOUND
		{
			return const_iterator(*this, GetBucketCount());
		}
		[[nodiscard]] const_iterator cend() const LIFETIME_BOUND
		{
			return const_iterator(*this, GetBucketCount());
		}

		[[nodiscard]] PURE_NOSTATICS static constexpr SizeType GetMaximumBucketCount()
		{
			return SizeType(1) << (sizeof(SizeType) * 8 - 1);
		}
		[[nodiscard]] PURE_NOSTATICS static constexpr SizeType GetTheoreticalCapacity()
		{
			return SizeType((uint64(GetMaximumBucketCount()) * MaximumLoadFactorNumerator) / MaximumLoadFactorDenominator);
		}
		/// Get the max load factor for this table (max number of elements / number of buckets)
		[[nodiscard]] PURE_NOSTATICS static constexpr float GetMaximumLoadFactor()
		{
			return float(MaximumLoadFactorNumerator) / float(MaximumLoadFactorDenominator);
		}

		[[nodiscard]] PURE_STATICS SizeType GetBucketCount() const noexcept
		{
			return ((m_allocator.GetCapacity() - 15) / (sizeof(KeyValue) + 1)) * (m_allocator.GetCapacity() > 0);
		}

		[[nodiscard]] PURE_STATICS constexpr bool IsEmpty() const
		{
			return m_size == 0;
		}
		[[nodiscard]] PURE_STATICS constexpr bool HasElements() const
		{
			return m_size != 0;
		}

		[[nodiscard]] PURE_STATICS constexpr SizeType GetSize() const
		{
			return m_size;
		}

		void Swap(THashTable& other) noexcept
		{
			ngine::Swap(m_allocator, other.m_allocator);
			ngine::Swap(m_size, other.m_size);
			ngine::Swap(m_remainingCapacity, other.m_remainingCapacity);
		}

		iterator Insert(const ValueType& value) LIFETIME_BOUND
		{
			const Key& key = HashTableDetail::GetKey(value);
			const uint64 keyHashValue = HashType{}(key);
			SizeType index;
			const bool wasInserted = InsertKey(key, keyHashValue, index);
			if (wasInserted)
			{
				new (Memory::GetAddressOf(GetKeyValues()[index])) KeyValue(value);
				return iterator(*this, index);
			}
			else
			{
				return end();
			}
		}
		template<typename ThisValueType = ValueType, typename = EnableIf<TypeTraits::IsCopyAssignable<ThisValueType>>>
		iterator InsertOrAssign(const ValueType& value) LIFETIME_BOUND
		{
			const Key& key = HashTableDetail::GetKey(value);
			const uint64 keyHashValue = HashType{}(key);
			iterator it = Find(key, keyHashValue);
			if (it != end())
			{
				it->second = value.second;
				return it;
			}
			else
			{
				SizeType index;
				const bool wasInserted = InsertKey(key, keyHashValue, index);
				if (wasInserted)
				{
					new (Memory::GetAddressOf(GetKeyValues()[index])) KeyValue(value);
					return iterator(*this, index);
				}
				else
				{
					return end();
				}
			}
		}
		iterator Emplace(ValueType&& value) LIFETIME_BOUND
		{
			const Key& key = HashTableDetail::GetKey(value);
			const uint64 keyHashValue = HashType{}(key);
			SizeType index;
			const bool wasInserted = InsertKey(key, keyHashValue, index);
			if (wasInserted)
			{
				new (Memory::GetAddressOf(GetKeyValues()[index])) KeyValue(Forward<ValueType>(value));
				return iterator(*this, index);
			}
			else
			{
				return end();
			}
		}
		template<typename ThisValueType = ValueType, typename = EnableIf<TypeTraits::IsMoveAssignable<ThisValueType>>>
		iterator EmplaceOrAssign(ValueType&& value) LIFETIME_BOUND
		{
			const Key& key = HashTableDetail::GetKey(value);
			const uint64 keyHashValue = HashType{}(key);
			iterator it = Find(key, keyHashValue);
			if (it != end())
			{
				it->second = Move(value.second);
				return it;
			}
			else
			{
				SizeType index;
				const bool wasInserted = InsertKey(key, keyHashValue, index);
				if (wasInserted)
				{
					new (Memory::GetAddressOf(GetKeyValues()[index])) KeyValue(Forward<ValueType>(value));
					return iterator(*this, index);
				}
				else
				{
					return end();
				}
			}
		}
		void Merge(THashTable& other)
		{
			Reserve(GetSize() + other.GetSize());
			for (auto otherIt = other.begin(), endIt = other.end(); otherIt != endIt; ++otherIt)
			{
				Emplace(Move(*otherIt));
			}
			other.Clear();
		}

		/// Find an element, returns iterator to element or end() if not found
		template<typename SearchedKeyType = Key>
		[[nodiscard]] iterator Find(const SearchedKeyType& key, const uint64 keyHashValue) LIFETIME_BOUND
		{
			// Check if we have any data
			if (IsEmpty())
			{
				return end();
			}

			// Split hash into index and control value
			SizeType index;
			uint8 control;
			GetBucketIndexAndControlValue(keyHashValue, index, control);

			const SizeType bucketCount = GetBucketCount();

			// Linear probing
			KeyEqual equal;
			const SizeType bucketMask = bucketCount - 1;
			const ControlMask control16{control};
			const ControlMask bucketEmpty{Math::Zero};
			const ControlView allControlBytes = GetControlBytes();
			const KeyValueView keyValues = GetKeyValues();
			for (;;)
			{
				// Read 16 control values
				// (note that we added 15 bytes at the end of the control values that mirror the first 15 bytes)
				Assert(index + 16 <= allControlBytes.GetSize());
				const ControlMask controlBytes{Math::Vectorization::LoadUnaligned, &allControlBytes[index]};

				// Check for the control value we're looking for
				// Note that when deleting we can create empty buckets instead of deleted buckets.
				// This means we must unconditionally check all buckets in this batch for equality
				// (also beyond the first empty bucket).
				uint32 controlEqual = uint32((controlBytes == control16).GetMask());

				// Index within the 16 buckets
				SizeType localIndex = index;

				// Loop while there's still buckets to process
				while (controlEqual != 0)
				{
					// Get the first equal bucket
					const uint32 firstEqual = Memory::GetNumberOfTrailingZeros(controlEqual);

					// Skip to the bucket
					localIndex += firstEqual;

					// Make sure that our index is not beyond the end of the table
					localIndex &= bucketMask;

					// We found a bucket with same control value
					const KeyValue& keyValue = keyValues[localIndex];
					if (equal(HashTableDetail::GetKey(keyValue), key))
					{
						// Element found
						return iterator(*this, localIndex);
					}

					// Skip past this bucket
					controlEqual >>= firstEqual + 1;
					localIndex++;
				}

				// Check for empty buckets
				const uint32 controlEmpty = uint32((controlBytes == bucketEmpty).GetMask());
				if (controlEmpty != 0)
				{
					// An empty bucket was found, we didn't find the element
					return end();
				}

				// Move to next batch of 16 buckets
				index = (index + 16) & bucketMask;
			}
		}
		template<typename SearchedKeyType = Key>
		[[nodiscard]] iterator Find(const SearchedKeyType& key) LIFETIME_BOUND
		{
			const uint64 keyHashValue = HashType{}(key);
			return Find<SearchedKeyType>(key, keyHashValue);
		}
		template<typename SearchedKeyType = Key>
		[[nodiscard]] const_iterator Find(const SearchedKeyType& key, const uint64 keyHashValue) const LIFETIME_BOUND
		{
			return const_cast<THashTable&>(*this).Find<SearchedKeyType>(key, keyHashValue);
		}
		template<typename SearchedKeyType = Key>
		[[nodiscard]] const_iterator Find(const SearchedKeyType& key) const LIFETIME_BOUND
		{
			const uint64 keyHashValue = HashType{}(key);
			return Find<SearchedKeyType>(key, keyHashValue);
		}

		template<typename SearchedKeyType = Key>
		[[nodiscard]] bool Contains(const SearchedKeyType& key, const uint64 keyHashValue) const
		{
			return Find<SearchedKeyType>(key, keyHashValue) != cend();
		}
		template<typename SearchedKeyType = Key>
		[[nodiscard]] bool Contains(const SearchedKeyType& key) const
		{
			const uint64 keyHashValue = HashType{}(key);
			return Contains(key, keyHashValue);
		}

		const_iterator Remove(const const_iterator iterator)
		{
			Assert(iterator.IsValid());

			// Read 16 control values before and after the current index
			// (note that we added 15 bytes at the end of the control values that mirror the first 15 bytes)
			const ControlView allControlBytes = GetControlBytes();
			const SizeType bucketCount = GetBucketCount();
			const SizeType controlBytesBeforeIndex = ((iterator.m_index - 16) & (bucketCount - 1));
			Assert(controlBytesBeforeIndex + 16 <= allControlBytes.GetSize());
			const ControlMask controlBytesBefore{Math::Vectorization::LoadUnaligned, &allControlBytes[controlBytesBeforeIndex]};
			Assert(iterator.m_index + 16 <= allControlBytes.GetSize());
			const ControlMask controlBytesAfter{Math::Vectorization::LoadUnaligned, &allControlBytes[iterator.m_index]};
			const ControlMask bucketEmpty{Math::Zero};
			const uint32 controlEmptyBefore = uint32((controlBytesBefore == bucketEmpty).GetMask());
			const uint32 controlEmptyAfter = uint32((controlBytesAfter == bucketEmpty).GetMask());

			// If (this index including) there exist 16 consecutive non-empty slots (represented by a bit being 0) then
			// a probe looking for some element needs to continue probing so we cannot mark the bucket as empty
			// but must mark it as deleted instead.
			// Note that we use: CountLeadingZeros(uint16) = CountLeadingZeros(uint32) - 16.
			const uint8 controlValue = Memory::GetNumberOfLeadingZeros(controlEmptyBefore) - 16 +
			                                 Memory::GetNumberOfTrailingZeros(controlEmptyAfter) <
			                               16
			                             ? EmptyBucketMask
			                             : DeletedBucketMask;

			// Mark the bucket as empty/deleted
			SetBucketControlValue(iterator.m_index, controlValue);

			if constexpr (!TypeTraits::IsTriviallyDestructible<KeyValue>)
			{
				GetKeyValues()[iterator.m_index].~KeyValue();
			}

			// If we marked the bucket as empty we can increase the load left
			if (controlValue == EmptyBucketMask)
			{
				++m_remainingCapacity;
			}

			// Decrease size
			--m_size;
			if (iterator.m_index < m_size)
			{
				return iterator;
			}
			else
			{
				return cend();
			}
		}
		iterator Remove(const iterator iteratorToRemove)
		{
			const const_iterator nextIterator = Remove(const_iterator{iteratorToRemove});

			return iterator{*this, nextIterator.m_index};
		}

		/// In place re-hashing of all elements in the table. Removes all DeletedBucketMask elements
		void Rehash()
		{
			const SizeType bucketCount = GetBucketCount();
			const ControlView allControlBytes = GetControlBytes();

			// Update the control value for all buckets
			for (SizeType i = 0; i < bucketCount; ++i)
			{
				uint8& control = allControlBytes[i];
				switch (control)
				{
					case DeletedBucketMask:
						// Deleted buckets become empty
						control = EmptyBucketMask;
						break;
					case EmptyBucketMask:
						// Remains empty
						break;
					default:
						// Mark all occupied as deleted, to indicate it needs to move to the correct place
						control = DeletedBucketMask;
						break;
				}
			}

			// Replicate control values to the last 15 entries
			for (SizeType i = 0; i < 15; ++i)
			{
				allControlBytes[bucketCount + i] = allControlBytes[i];
			}

			// Loop over all elements that have been 'deleted' and move them to their new spot
			const ControlMask bucketUsed{UsedBucketMask};
			const SizeType bucketMask = bucketCount - 1;
			const uint32 probeMask = bucketMask & ~uint32(0b1111); // Mask out lower 4 bits because we test 16 buckets at a time

			const KeyValueView keyValues = GetKeyValues();
			for (SizeType sourceBucketIndex = 0; sourceBucketIndex < bucketCount; ++sourceBucketIndex)
			{
				if (allControlBytes[sourceBucketIndex] == DeletedBucketMask)
				{
					for (;;)
					{
						// Split hash into index and control value
						SizeType sourceIndex;
						uint8 sourceControl;

						const Key& key = HashTableDetail::GetKey(keyValues[sourceBucketIndex]);
						const uint64 keyHashValue = HashType{}(key);
						GetBucketIndexAndControlValue(keyHashValue, sourceIndex, sourceControl);

						// Linear probing
						SizeType destinationBucketIndex = sourceIndex;
						for (;;)
						{
							// Check if any buckets are free
							Assert(destinationBucketIndex + 16 <= allControlBytes.GetSize());
							const ControlMask controlBytes{Math::Vectorization::LoadUnaligned, &allControlBytes[destinationBucketIndex]};
							const uint32 controlFree = uint32((controlBytes & bucketUsed).GetMask()) ^ 0xffff;
							if (controlFree != 0)
							{
								// Select this bucket as destination
								destinationBucketIndex += Memory::GetNumberOfTrailingZeros(controlFree);
								destinationBucketIndex &= bucketMask;
								break;
							}

							// Move to next batch of 16 buckets
							destinationBucketIndex = (destinationBucketIndex + 16) & bucketMask;
						}

						// Check if we stay in the same probe group
						if (((destinationBucketIndex - sourceIndex) & probeMask) == ((sourceBucketIndex - sourceIndex) & probeMask))
						{
							// We stay in the same group, we can stay where we are
							SetBucketControlValue(sourceBucketIndex, sourceControl);
							break;
						}
						else if (allControlBytes[destinationBucketIndex] == EmptyBucketMask)
						{
							// There's an empty bucket, move us there
							SetBucketControlValue(destinationBucketIndex, sourceControl);
							SetBucketControlValue(sourceBucketIndex, EmptyBucketMask);
							new (Memory::GetAddressOf(keyValues[destinationBucketIndex])) KeyValue(Move(keyValues[sourceBucketIndex]));
							if constexpr (!TypeTraits::IsTriviallyDestructible<KeyValue>)
							{
								keyValues[sourceBucketIndex].~KeyValue();
							}
							break;
						}
						else
						{
							// There's an element in the bucket we want to move to, swap them
							Assert(allControlBytes[destinationBucketIndex] == DeletedBucketMask);
							SetBucketControlValue(destinationBucketIndex, sourceControl);
							if constexpr (TypeTraits::IsMoveAssignable<KeyValue>)
							{
								KeyValue sourceData = Move(keyValues[sourceBucketIndex]);
								keyValues[sourceBucketIndex] = Move(keyValues[destinationBucketIndex]);
								keyValues[destinationBucketIndex] = Move(sourceData);
							}
							else
							{
								static_assert(TypeTraits::IsMoveConstructible<KeyValue>);
								KeyValue sourceData{Move(keyValues[sourceBucketIndex])};
								keyValues[sourceBucketIndex].~KeyValue();
								new (Memory::GetAddressOf(keyValues[sourceBucketIndex])) KeyValue(Move(keyValues[destinationBucketIndex]));
								keyValues[destinationBucketIndex].~KeyValue();
								new (Memory::GetAddressOf(keyValues[destinationBucketIndex])) KeyValue(Move(sourceData));
							}
							// Iterate again with the same source bucket
						}
					}
				}
			}

			// Reinitialize load left
			m_remainingCapacity = GetMaximumElementCount(bucketCount) - m_size;
		}
	private:
		/// Max load factor is MaximumLoadFactorNumerator / MaximumLoadFactorDenominator
		inline static constexpr SizeType MaximumLoadFactorNumerator = 7;
		inline static constexpr SizeType MaximumLoadFactorDenominator = 8;

		/// If we can recover this fraction of deleted elements, we'll reshuffle the buckets in place rather than growing the table
		inline static constexpr SizeType MaximumDeletedElementsNumerator = 1;
		inline static constexpr SizeType MaximumDeletedElementsDenominator = 8;

		/// Values that the control bytes can have
		inline static constexpr uint8 EmptyBucketMask = 0;
		inline static constexpr uint8 DeletedBucketMask = 0x7f;
		inline static constexpr uint8 UsedBucketMask = 0x80; // Lowest 7 bits are lowest 7 bits of the hash value

		[[nodiscard]] PURE_STATICS KeyValueView GetKeyValues() noexcept LIFETIME_BOUND
		{
			return KeyValueView{reinterpret_cast<KeyValue*>(m_allocator.GetData()), GetBucketCount()};
		}
		[[nodiscard]] PURE_STATICS ConstKeyValueView GetKeyValues() const noexcept LIFETIME_BOUND
		{
			return ConstKeyValueView{reinterpret_cast<const KeyValue*>(m_allocator.GetData()), GetBucketCount()};
		}

		[[nodiscard]] PURE_STATICS ControlView GetControlBytes() noexcept LIFETIME_BOUND
		{
			const SizeType bucketCount = GetBucketCount();
			return ControlView{
				reinterpret_cast<uint8*>(reinterpret_cast<KeyValue*>(m_allocator.GetData()) + bucketCount),
				bucketCount + 15 * (bucketCount > 0)
			};
		}
		[[nodiscard]] PURE_STATICS ConstControlView GetControlBytes() const noexcept LIFETIME_BOUND
		{
			const SizeType bucketCount = GetBucketCount();
			return ConstControlView{
				reinterpret_cast<const uint8*>(reinterpret_cast<const KeyValue*>(m_allocator.GetData()) + bucketCount),
				bucketCount + 15 * (bucketCount > 0)
			};
		}
	private:
		AllocatorType m_allocator;
		SizeType m_size = 0;
		SizeType m_remainingCapacity = 0;
	};
}

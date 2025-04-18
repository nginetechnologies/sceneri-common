#pragma once

#include <Common/Platform/Pure.h>
#include <Common/Memory/New.h>
#include <Common/Memory/Pair.h>
#include <Common/Memory/Containers/HashTable.h>
#include <Common/Memory/Allocators/DynamicAllocator.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/EnableIf.h>

#define ENABLE_HASH_TABLE_UNORDERED_SET 0

PUSH_MSVC_WARNINGS_TO_LEVEL(2)
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated-builtins")
DISABLE_CLANG_WARNING("-Wdeprecated")

#if ENABLE_HASH_TABLE_UNORDERED_SET
#include <Common/3rdparty/absl/hash/hash.h>
#else
#include <Common/3rdparty/absl/container/flat_hash_set.h>
#endif

POP_CLANG_WARNINGS
POP_MSVC_WARNINGS

namespace ngine
{
#if ENABLE_HASH_TABLE_UNORDERED_SET
	namespace Internal
	{
		template<typename Key>
		struct UnorderedSetDetail
		{
			[[nodiscard]] static constexpr const Key& GetKey(const Key& key)
			{
				return key;
			}
		};
	}
#endif

	template<
		typename _KeyType,
		typename HashType = absl::Hash<_KeyType>,
		typename EqualityType = Memory::Internal::DefaultEqualityCheck<_KeyType>>
#if ENABLE_HASH_TABLE_UNORDERED_SET
	struct UnorderedSet : private THashTable<
													_KeyType,
													_KeyType,
													Internal::UnorderedSetDetail<_KeyType>,
													HashType,
													EqualityType,
													Memory::DynamicAllocator<ByteType, uint32, uint32>>
#else
	struct UnorderedSet
#endif
	{
		using KeyType = _KeyType;
#if ENABLE_HASH_TABLE_UNORDERED_SET
		using BaseType = THashTable<
			_KeyType,
			_KeyType,
			Internal::UnorderedSetDetail<_KeyType>,
			HashType,
			EqualityType,
			Memory::DynamicAllocator<ByteType, uint32, uint32>>;
#else
		using SetType = absl::flat_hash_set<KeyType, HashType, EqualityType>;
#endif
	public:
		UnorderedSet() = default;
		UnorderedSet(const UnorderedSet&) = default;
		UnorderedSet& operator=(const UnorderedSet&) = default;
		UnorderedSet(UnorderedSet&&) = default;
		UnorderedSet& operator=(UnorderedSet&&) = default;
		template<typename... KeyTypes>
		UnorderedSet(KeyTypes&&... keys)
		{
			Reserve(sizeof...(keys));
			(Emplace(Move(keys)), ...);
		}
		UnorderedSet(Memory::ReserveType, const uint32 size)
#if ENABLE_HASH_TABLE_UNORDERED_SET
			: BaseType(Memory::Reserve, size)
#else
			: m_set(size)
#endif
		{
		}
		~UnorderedSet() = default;

#if ENABLE_HASH_TABLE_UNORDERED_SET
		using iterator = typename BaseType::iterator;
		using const_iterator = typename BaseType::const_iterator;
#else
		using iterator = typename SetType::iterator;
		using const_iterator = typename SetType::const_iterator;
#endif

#if ENABLE_HASH_TABLE_UNORDERED_SET
		using BaseType::begin;
		using BaseType::cbegin;
		using BaseType::end;
		using BaseType::cend;

		using BaseType::HasElements;
		using BaseType::IsEmpty;
		using BaseType::GetSize;
		using BaseType::GetTheoreticalCapacity;
#else
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr iterator begin() LIFETIME_BOUND
		{
			return m_set.begin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator begin() const LIFETIME_BOUND
		{
			return m_set.begin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator cbegin() const LIFETIME_BOUND
		{
			return m_set.cbegin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr iterator end() LIFETIME_BOUND
		{
			return m_set.end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator end() const LIFETIME_BOUND
		{
			return m_set.end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator cend() const LIFETIME_BOUND
		{
			return m_set.cend();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool HasElements() const
		{
			return !m_set.empty();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsEmpty() const
		{
			return m_set.empty();
		}
#endif

		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyType& key) LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_SET
			return BaseType::Find(key, hash(key));
#else
			return m_set.find(key, hash(key));
#endif
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyType& key) const LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_SET
			return BaseType::Find(key, hash(key));
#else
			return m_set.find(key, hash(key));
#endif
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyType& key) const
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_SET
			return BaseType::Contains(key, hash(key));
#else
			return m_set.find(key, hash(key)) != m_set.end();
#endif
		}

		inline static constexpr bool IsTransparent = Internal::IsTransparent<HashType> && Internal::IsTransparent<EqualityType>;

		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyComparableType key) LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_SET
			return BaseType::template Find<KeyComparableType>(key, hash(key));
#else
			return m_set.template find<KeyComparableType>(key, hash(key));
#endif
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyComparableType key) const LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_SET
			return BaseType::template Find<KeyComparableType>(key, hash(key));
#else
			return m_set.template find<KeyComparableType>(key, hash(key));
#endif
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyComparableType key) const
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_SET
			return BaseType::template Contains<KeyComparableType>(key, hash(key));
#else
			return m_set.template find<KeyComparableType>(key, hash(key)) != m_set.end();
#endif
		}

#if ENABLE_HASH_TABLE_UNORDERED_SET
		using BaseType::Insert;
		using BaseType::Emplace;

		using BaseType::Clear;
		using BaseType::Remove;
		using BaseType::Reserve;
		using BaseType::Merge;
#else
		FORCE_INLINE iterator Emplace(KeyType&& key) LIFETIME_BOUND
		{
			return m_set.emplace(typename SetType::value_type(Forward<KeyType>(key))).first;
		}
		FORCE_INLINE iterator Insert(const KeyType key) LIFETIME_BOUND
		{
			return m_set.insert(typename SetType::value_type(key)).first;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS uint32 GetSize() const
		{
			return (uint32)m_set.size();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS uint32 GetCapacity() const
		{
			return (uint32)m_set.capacity();
		}
		FORCE_INLINE void Clear()
		{
			m_set.clear();
		}
		FORCE_INLINE const_iterator Remove(const_iterator it)
		{
			m_set.erase(it);
			return it++;
		}
		FORCE_INLINE void Reserve(const uint32 size)
		{
			m_set.reserve(size);
		}

		FORCE_INLINE void Merge(SetType& otherSet)
		{
			m_set.merge(otherSet);
		}
	protected:
		SetType m_set;
#endif
	};
}

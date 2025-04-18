#pragma once

#include <Common/Platform/Pure.h>
#include <Common/Math/Hash.h>
#include <Common/Memory/New.h>
#include <Common/Memory/Pair.h>
#include <Common/Memory/Containers/HashTable.h>
#include <Common/Memory/Allocators/DynamicAllocator.h>
#include <Common/Memory/Containers/ForwardDeclarations/UnorderedMap.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/EnableIf.h>

#define ENABLE_HASH_TABLE_UNORDERED_MAP 0

PUSH_MSVC_WARNINGS_TO_LEVEL(2)
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated-builtins")
DISABLE_CLANG_WARNING("-Wdeprecated")

#if ENABLE_HASH_TABLE_UNORDERED_MAP
#include <Common/3rdparty/absl/hash/hash.h>
#else
#include <Common/3rdparty/absl/container/flat_hash_map.h>
#endif

POP_CLANG_WARNINGS
POP_MSVC_WARNINGS

namespace ngine
{
#if ENABLE_HASH_TABLE_UNORDERED_MAP
	namespace Internal
	{
		template<typename KeyType, typename ValueType>
		struct TRIVIAL_ABI LegacyPair
		{
			KeyType first;
			ValueType second;
		};

		template<typename Key, typename Value>
		struct UnorderedMapDetail
		{
			[[nodiscard]] static constexpr const Key& GetKey(const LegacyPair<Key, Value>& keyValuePair)
			{
				return keyValuePair.first;
			}
		};
	}
#endif

	template<typename _KeyType, typename _ValueType, typename HashType, typename EqualityType>
#if ENABLE_HASH_TABLE_UNORDERED_MAP
	struct UnorderedMap : private THashTable<
													_KeyType,
													Internal::LegacyPair<_KeyType, _ValueType>,
													Internal::UnorderedMapDetail<_KeyType, _ValueType>,
													HashType,
													EqualityType,
													Memory::DynamicAllocator<ByteType, uint32, uint32>>
#else
	struct UnorderedMap
#endif
	{
		using KeyType = _KeyType;
		using ValueType = _ValueType;
		using NewPairType = Pair<KeyType, ValueType>;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
		using BaseType = THashTable<
			_KeyType,
			Internal::LegacyPair<_KeyType, _ValueType>,
			Internal::UnorderedMapDetail<_KeyType, _ValueType>,
			HashType,
			EqualityType,
			Memory::DynamicAllocator<ByteType, uint32, uint32>>;
		using PairType = Internal::LegacyPair<KeyType, ValueType>;
#else
		using MapType = absl::flat_hash_map<KeyType, ValueType, HashType, EqualityType>;
		using PairType = std::pair<const KeyType, ValueType>;
#endif
	public:
		UnorderedMap() = default;
		UnorderedMap(const UnorderedMap&) = default;
		UnorderedMap& operator=(const UnorderedMap&) = default;
		UnorderedMap(UnorderedMap&&) = default;
		UnorderedMap& operator=(UnorderedMap&&) = default;
		template<typename... PairTypes>
		explicit UnorderedMap(PairTypes&&... pairs)
		{
			Reserve(sizeof...(pairs));
			(Emplace(Move(pairs.key), Move(pairs.value)), ...);
		}
		UnorderedMap(Memory::ReserveType, const uint32 size)
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			: BaseType(Memory::Reserve, size)
#else
			: m_map(size)
#endif
		{
		}
		~UnorderedMap() = default;

#if ENABLE_HASH_TABLE_UNORDERED_MAP
		using iterator = typename BaseType::iterator;
		using const_iterator = typename BaseType::const_iterator;
#else
		using iterator = typename MapType::iterator;
		using const_iterator = typename MapType::const_iterator;
#endif

#if ENABLE_HASH_TABLE_UNORDERED_MAP
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
			return m_map.begin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator begin() const LIFETIME_BOUND
		{
			return m_map.begin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator cbegin() const LIFETIME_BOUND
		{
			return m_map.cbegin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr iterator end() LIFETIME_BOUND
		{
			return m_map.end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator end() const LIFETIME_BOUND
		{
			return m_map.end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator cend() const LIFETIME_BOUND
		{
			return m_map.cend();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool HasElements() const
		{
			return !m_map.empty();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsEmpty() const
		{
			return m_map.empty();
		}
#endif

		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyType& key) LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::Find(key, hash(key));
#else
			return m_map.find(key, hash(key));
#endif
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyType& key) const LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::Find(key, hash(key));
#else
			return m_map.find(key, hash(key));
#endif
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyType& key) const
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::Contains(key, hash(key));
#else
			return m_map.find(key, hash(key)) != m_map.end();
#endif
		}

		inline static constexpr bool IsTransparent = Internal::IsTransparent<HashType> && Internal::IsTransparent<EqualityType>;

		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyComparableType key) LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::template Find<KeyComparableType>(key, hash(key));
#else
			return m_map.template find<KeyComparableType>(key, hash(key));
#endif
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyComparableType key) const LIFETIME_BOUND
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::template Find<KeyComparableType>(key, hash(key));
#else
			return m_map.template find<KeyComparableType>(key, hash(key));
#endif
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyComparableType key) const
		{
			HashType hash;
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::template Contains<KeyComparableType>(key, hash(key));
#else
			return m_map.template find<KeyComparableType>(key, hash(key)) != m_map.end();
#endif
		}

		//! Emplaces a key and its value into the map, assuming that the key does not already exist.
		FORCE_INLINE iterator Emplace(KeyType&& key, ValueType&& value) LIFETIME_BOUND
		{
			Assert(!Contains(key), "Silently failing insertion into map!");
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::Emplace(PairType{Forward<KeyType>(key), Forward<ValueType>(value)});
#else
			return m_map.emplace(typename MapType::value_type(Forward<KeyType>(key), Forward<ValueType>(value))).first;
#endif
		}
		//! Emplaces a key and its value into the map, assuming that the key does not already exist.
		template<typename KeyImplicitType, typename ValueImplicitType>
		FORCE_INLINE iterator Emplace(KeyImplicitType&& key, ValueImplicitType&& value) LIFETIME_BOUND
		{
			Assert(!Contains(key), "Silently failing insertion into map!");
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::Emplace(PairType{KeyType(Forward<KeyImplicitType>(key)), ValueType(Forward<ValueImplicitType>(value))});
#else
			return m_map.emplace(typename MapType::value_type(Forward<KeyImplicitType>(key), Forward<ValueImplicitType>(value))).first;
#endif
		}
		//! Emplaces a key and its value into the map, replacing the current key and value if it existed.
		FORCE_INLINE iterator EmplaceOrAssign(KeyType&& key, ValueType&& value) LIFETIME_BOUND
		{
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::EmplaceOrAssign(PairType{Forward<KeyType>(key), Forward<ValueType>(value)});
#else
			return m_map.insert_or_assign(Forward<KeyType>(key), Forward<ValueType>(value)).first;
#endif
		}
		//! Emplaces a key and its value into the map, replacing the current key and value if it existed.
		template<typename KeyImplicitType, typename ValueImplicitType>
		FORCE_INLINE iterator EmplaceOrAssign(KeyImplicitType&& key, ValueImplicitType&& value) LIFETIME_BOUND
		{
#if ENABLE_HASH_TABLE_UNORDERED_MAP
			return BaseType::template EmplaceOrAssign(
				PairType{KeyType(Forward<KeyImplicitType>(key)), ValueType(Forward<ValueImplicitType>(value))}
			);
#else
			return m_map.insert_or_assign(Forward<KeyImplicitType>(key), Forward<ValueImplicitType>(value)).first;
#endif
		}

#if ENABLE_HASH_TABLE_UNORDERED_MAP
		using BaseType::Clear;
		using BaseType::Remove;
		using BaseType::Reserve;

		FORCE_INLINE void Merge(UnorderedMap& otherMap)
		{
			BaseType::Merge(otherMap);
		}
#else
		[[nodiscard]] FORCE_INLINE PURE_STATICS uint32 GetSize() const
		{
			return (uint32)m_map.size();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS uint32 GetCapacity() const
		{
			return (uint32)m_map.capacity();
		}

		FORCE_INLINE void Clear()
		{
			m_map.clear();
		}
		FORCE_INLINE const_iterator Remove(const_iterator it)
		{
			m_map.erase(it);
			return it++;
		}
		FORCE_INLINE iterator Remove(iterator it)
		{
			m_map.erase(it);
			return ++it;
		}
		FORCE_INLINE void Reserve(const uint32 size)
		{
			m_map.reserve(size);
		}

		//! Merges items from other into this, moving from the source.
		//! If the item is already present in this it will remain in source.
		FORCE_INLINE void Merge(UnorderedMap& otherMap)
		{
			m_map.merge(otherMap.m_map);
		}
	protected:
		MapType m_map;
#endif
	};
}

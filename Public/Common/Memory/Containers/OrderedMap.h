#pragma once

#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/Pure.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/New.h>
#include <Common/Memory/Pair.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/EnableIf.h>

PUSH_MSVC_WARNINGS_TO_LEVEL(2)
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated-builtins")
DISABLE_CLANG_WARNING("-Wdeprecated")

#include <Common/3rdparty/absl/container/btree_map.h>

POP_CLANG_WARNINGS
POP_MSVC_WARNINGS

#include <Common/Platform/ForceInline.h>

namespace ngine
{
	namespace Internal
	{
		template<typename Type>
		struct DefaultLessCheck
		{
			using is_transparent = void;

			template<typename LeftType, typename RightType>
			bool operator()(const LeftType& leftType, const RightType& rightType) const
			{
				return leftType < rightType;
			}
		};
	}

	template<typename _KeyType, typename _ValueType, typename Compare = Internal::DefaultLessCheck<_KeyType>>
	struct Orderedmap
	{
		using KeyType = _KeyType;
		using ValueType = _ValueType;
		using PairType = std::pair<const KeyType, ValueType>;
		using NewPairType = Pair<KeyType, ValueType>;
	protected:
		using MapType = absl::btree_map<KeyType, ValueType, Compare>;
	public:
		OrderedMap() = default;
		OrderedMap(const OrderedMap&) = default;
		OrderedMap& operator=(const OrderedMap&) = default;
		OrderedMap(OrderedMap&&) = default;
		OrderedMap& operator=(OrderedMap&&) = default;
		template<typename... PairTypes>
		OrderedMap(PairTypes&&... pairs)
		{
			Reserve(sizeof...(pairs));
			(Emplace(Move(pairs.key), Move(pairs.value)), ...);
		}
		OrderedMap(Memory::ReserveType, const uint32 size)
			: m_map(size)
		{
		}
		~OrderedMap() = default;

		using iterator = typename MapType::iterator;
		using const_iterator = typename MapType::const_iterator;

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

		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyType& key) LIFETIME_BOUND
		{
			HashType hash;
			return m_map.find(key, hash(key));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyType& key) const LIFETIME_BOUND
		{
			HashType hash;
			return m_map.find(key, hash(key));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyType& key) const
		{
			HashType hash;
			return m_map.find(key, hash(key)) != m_map.end();
		}

		template<typename Type>
		inline static constexpr bool TIsTransparent = absl::container_internal::IsTransparent<Type>::value;

		inline static constexpr bool IsTransparent = TIsTransparent<HashType> && TIsTransparent<EqualityType>;

		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyComparableType key) LIFETIME_BOUND
		{
			HashType hash;
			return m_map.template find<KeyComparableType>(key, hash(key));
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyComparableType key) const LIFETIME_BOUND
		{
			HashType hash;
			return m_map.template find<KeyComparableType>(key, hash(key));
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyComparableType key) const
		{
			HashType hash;
			return m_map.template find<KeyComparableType>(key, hash(key)) != m_map.end();
		}

		FORCE_INLINE iterator Emplace(KeyType&& key, ValueType&& value) LIFETIME_BOUND
		{
			return m_map.emplace(typename MapType::value_type(Forward<KeyType>(key), Forward<ValueType>(value))).first;
		}
		template<typename KeyImplicitType, typename ValueImplicitType>
		FORCE_INLINE iterator Emplace(KeyImplicitType&& key, ValueImplicitType&& value) LIFETIME_BOUND
		{
			return m_map.emplace(typename MapType::value_type(Forward<KeyImplicitType>(key), Forward<ValueImplicitType>(value))).first;
		}
		FORCE_INLINE iterator Insert(const KeyType key, const ValueType value) LIFETIME_BOUND
		{
			return m_map.insert(typename MapType::value_type(key, value)).first;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS uint32 GetSize() const
		{
			return (uint32)m_map.size();
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
		FORCE_INLINE void Merge(const MapType& otherMap)
		{
			m_map.merge(otherMap);
		}
	protected:
		MapType m_map;
	};
}

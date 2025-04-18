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
DISABLE_CLANG_WARNING("-Wimplicit-int-conversion")
DISABLE_CLANG_WARNING("-Wshorten-64-to-32")
DISABLE_CLANG_WARNING("-Wsign-compare")

PUSH_GCC_WARNINGS
DISABLE_GCC_WARNING("-Wsign-compare")

#include <Common/3rdparty/absl/container/btree_set.h>

POP_CLANG_WARNINGS
POP_GCC_WARNINGS
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

	template<typename _KeyType, typename Compare = Internal::DefaultLessCheck<_KeyType>>
	struct OrderedSet
	{
		using KeyType = _KeyType;
	protected:
		using SetType = absl::btree_set<KeyType, Compare>;
	public:
		OrderedSet() = default;
		OrderedSet(const OrderedSet&) = default;
		OrderedSet& operator=(const OrderedSet&) = default;
		OrderedSet(OrderedSet&&) = default;
		OrderedSet& operator=(OrderedSet&&) = default;
		template<typename... KeyTypes>
		OrderedSet(KeyTypes&&... keys)
		{
			(Emplace(Move(keys)), ...);
		}
		OrderedSet(Memory::ReserveType, const uint32 size)
			: m_set(size)
		{
		}
		~OrderedSet() = default;

		using iterator = typename SetType::iterator;
		using const_iterator = typename SetType::const_iterator;

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

		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyType& key) LIFETIME_BOUND
		{
			return m_set.find(key);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyType& key) const LIFETIME_BOUND
		{
			return m_set.find(key);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyType& key) const
		{
			return m_set.find(key) != m_set.end();
		}

		template<typename Type>
		inline static constexpr bool TIsTransparent = absl::container_internal::IsTransparent<Type>::value;

		inline static constexpr bool IsTransparent = TIsTransparent<Compare>;

		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS iterator Find(const KeyComparableType key) LIFETIME_BOUND
		{
			return m_set.template find<KeyComparableType>(key);
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS const_iterator Find(const KeyComparableType key) const LIFETIME_BOUND
		{
			return m_set.template find<KeyComparableType>(key);
		}
		template<typename KeyComparableType, bool Enable = IsTransparent, typename = EnableIf<Enable>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const KeyComparableType key) const
		{
			return m_set.template find<KeyComparableType>(key) != m_set.end();
		}

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

		FORCE_INLINE void Clear()
		{
			m_set.clear();
		}
		FORCE_INLINE const_iterator Remove(const_iterator it)
		{
			m_set.erase(it);
			return it++;
		}
		FORCE_INLINE void Merge(const SetType& otherMap)
		{
			m_set.merge(otherMap);
		}
	protected:
		SetType m_set;
	};
}

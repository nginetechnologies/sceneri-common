#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Memory/Forward.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/TrivialABI.h>

#include <list>

namespace ngine
{
	template<class Type>
	class TRIVIAL_ABI DoubleLinkedList
	{
	protected:
		using ListType = std::list<Type>;
	public:
		using iterator = typename ListType::iterator;
		using const_iterator = typename ListType::const_iterator;
		using reverse_iterator = typename ListType::reverse_iterator;
		using const_reverse_iterator = typename ListType::const_reverse_iterator;
	public:
		DoubleLinkedList() = default;
		DoubleLinkedList(const DoubleLinkedList&) = delete;
		DoubleLinkedList& operator=(const DoubleLinkedList&) = delete;
		DoubleLinkedList(DoubleLinkedList&&) = default;
		DoubleLinkedList& operator=(DoubleLinkedList&&) = default;
		~DoubleLinkedList() = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr iterator begin() LIFETIME_BOUND
		{
			return m_list.begin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator begin() const LIFETIME_BOUND
		{
			return m_list.begin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator cbegin() const LIFETIME_BOUND
		{
			return m_list.cbegin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr reverse_iterator rbegin() LIFETIME_BOUND
		{
			return m_list.rbegin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_reverse_iterator rbegin() const LIFETIME_BOUND
		{
			return m_list.rbegin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_reverse_iterator crbegin() const LIFETIME_BOUND
		{
			return m_list.crbegin();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr iterator end() LIFETIME_BOUND
		{
			return m_list.end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator end() const LIFETIME_BOUND
		{
			return m_list.end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_iterator cend() const LIFETIME_BOUND
		{
			return m_list.cend();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr reverse_iterator rend() LIFETIME_BOUND
		{
			return m_list.rend();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_reverse_iterator rend() const LIFETIME_BOUND
		{
			return m_list.rend();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const_reverse_iterator crend() const LIFETIME_BOUND
		{
			return m_list.crend();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool HasElements() const
		{
			return !m_list.empty();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsEmpty() const
		{
			return m_list.empty();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS uint32 GetSize() const
		{
			return (uint32)m_list.size();
		}
		FORCE_INLINE void Clear()
		{
			m_list.clear();
		}
		FORCE_INLINE void EmplaceBack(Type&& value)
		{
			m_list.emplace_back(Forward<Type>(value));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS Type& GetFirstElement() LIFETIME_BOUND
		{
			return m_list.front();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS const Type& GetFirstElement() const LIFETIME_BOUND
		{
			return m_list.front();
		}
	private:
		ListType m_list;
	};
}

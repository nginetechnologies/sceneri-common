#pragma once

#include <Common/Memory/Optional.h>

namespace ngine
{
	template<typename Type, bool Reverse = false>
	struct Iterator
	{
		using difference_type = ptrdiff;
		using value_type = Type;
		using reference = Type&;
		using pointer = Type*;

		FORCE_INLINE constexpr Iterator() = default;
		FORCE_INLINE constexpr Iterator(Type* pIterator) noexcept
			: m_pIterator(pIterator)
		{
		}
		template<typename ElementType = Type, bool IsReversed = Reverse, typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE constexpr Iterator(const Iterator<TypeTraits::WithoutConst<Type>, IsReversed>& other) noexcept
			: m_pIterator(other)
		{
		}
		constexpr Iterator(const Iterator&) = default;
		constexpr Iterator(Iterator&&) = default;
		constexpr Iterator& operator=(const Iterator&) = default;
		constexpr Iterator& operator=(Iterator&&) = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator Type*() const noexcept
		{
			return m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type* Get() const noexcept
		{
			return m_pIterator;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS Type* operator->() const noexcept
		{
			return m_pIterator;
		}

		FORCE_INLINE constexpr Iterator operator++() noexcept
		{
			if constexpr (Reverse)
			{
				return Iterator{--m_pIterator};
			}
			else
			{
				return Iterator{++m_pIterator};
			}
		}
		FORCE_INLINE constexpr Iterator operator++(int) noexcept
		{
			if constexpr (Reverse)
			{
				return Iterator{m_pIterator--};
			}
			else
			{
				return Iterator{m_pIterator++};
			}
		}
		FORCE_INLINE constexpr Iterator& operator+=(const int64 value) noexcept
		{
			if constexpr (Reverse)
			{
				m_pIterator -= value;
			}
			else
			{
				m_pIterator += value;
			}
			return *this;
		}

		FORCE_INLINE constexpr Iterator operator--() noexcept
		{
			if constexpr (Reverse)
			{
				return Iterator{++m_pIterator};
			}
			else
			{
				return Iterator{--m_pIterator};
			}
		}
		FORCE_INLINE constexpr Iterator operator--(int) noexcept
		{
			if constexpr (Reverse)
			{
				return Iterator{m_pIterator++};
			}
			else
			{
				return Iterator{m_pIterator--};
			}
		}
		FORCE_INLINE constexpr Iterator& operator-=(const int64 value) noexcept
		{
			if constexpr (Reverse)
			{
				m_pIterator += value;
			}
			else
			{
				m_pIterator -= value;
			}
			return *this;
		}

		template<typename OtherType = Type, bool IsReversed = Reverse>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Iterator<OtherType, IsReversed> other) const noexcept
		{
			return other.Get() == m_pIterator;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Type* const pOther) const noexcept
		{
			return pOther == m_pIterator;
		}

		template<typename ElementType = Type, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(Type* const pOther) const noexcept
		{
			return pOther == m_pIterator;
		}
	protected:
		Type* m_pIterator;
	};

	template<typename Type, bool IsReversed>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Type* const left, const Iterator<Type, IsReversed> right) noexcept
	{
		return right == left;
	}

	template<typename Type, bool IsReversed, typename = EnableIf<!TypeTraits::IsConst<Type>>>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(Type* const left, const Iterator<Type, IsReversed> right) noexcept
	{
		return right == left;
	}
}

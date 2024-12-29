#pragma once

#include <Common/Memory/Iterator.h>
#include <Common/Memory/Optional.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename Type, bool Reverse>
	struct TRIVIAL_ABI Optional<Iterator<Type, Reverse>>
	{
		using IteratorType = Iterator<Type, Reverse>;

		constexpr Optional() = default;
		constexpr Optional(Type* pIterator, const Type* pEnd) noexcept
			: m_pIterator(pIterator)
			, m_pEnd(pEnd)
		{
		}
		Optional(Optional&&) = default;
		Optional(const Optional&) = default;
		Optional& operator=(Optional&&) = default;
		Optional& operator=(const Optional&) = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type* Get() const noexcept
		{
			return m_pIterator != m_pEnd ? m_pIterator : nullptr;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type& GetReference() const noexcept
		{
			Expect(m_pIterator != m_pEnd);
			return *m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type& GetUnchecked() const noexcept
		{
			return *m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsValid() const noexcept
		{
			return m_pIterator != m_pEnd;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS Type* operator->() const noexcept
		{
			Expect(m_pIterator != m_pEnd);
			return m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS Type& operator*() const noexcept
		{
			Expect(m_pIterator != m_pEnd);
			return *m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS operator Type*() const noexcept LIFETIME_BOUND
		{
			return m_pIterator != m_pEnd ? m_pIterator : nullptr;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator bool() const noexcept
		{
			return m_pIterator != m_pEnd;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator IteratorType() const noexcept
		{
			return IteratorType(m_pIterator);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Optional& other) const noexcept
		{
			return m_pIterator == other.m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const Optional& other) const noexcept
		{
			return m_pIterator != other.m_pIterator;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const IteratorType& pOther) const noexcept
		{
			return m_pIterator == pOther;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const IteratorType& pOther) const noexcept
		{
			return m_pIterator != pOther;
		}
	protected:
		Type* m_pIterator = nullptr;
		const Type* m_pEnd = nullptr;
	};

	template<typename Type>
	using OptionalIterator = Optional<Iterator<Type, false>>;
	template<typename Type>
	using OptionalReverseIterator = Optional<Iterator<Type, true>>;
}

#pragma once

#include "EnumFlags.h"
#include <Common/Threading/AtomicInteger.h>

namespace ngine
{
	template<typename EnumType>
	struct AtomicEnumFlags
	{
		using UnderlyingType = TypeTraits::UnderlyingType<EnumType>;
		using EnumFlagsType = EnumFlags<EnumType>;

		FORCE_INLINE constexpr AtomicEnumFlags(const EnumFlagsType value)
			: m_flags(value.GetUnderlyingValue())
		{
		}
		FORCE_INLINE constexpr AtomicEnumFlags(const EnumType value)
			: m_flags((UnderlyingType)value)
		{
		}
		FORCE_INLINE constexpr AtomicEnumFlags() = default;
		AtomicEnumFlags(const AtomicEnumFlags&) = default;
		AtomicEnumFlags& operator=(const AtomicEnumFlags&) = default;
		AtomicEnumFlags(AtomicEnumFlags&&) = default;
		AtomicEnumFlags& operator=(AtomicEnumFlags&&) = default;
		FORCE_INLINE AtomicEnumFlags& operator=(const EnumFlagsType other)
		{
			m_flags = other.GetUnderlyingValue();
			return *this;
		}
	protected:
		FORCE_INLINE explicit AtomicEnumFlags(const UnderlyingType value)
			: m_flags(static_cast<EnumType>(value))
		{
		}
	public:
		[[nodiscard]] FORCE_INLINE bool IsEmpty() const
		{
			return GetUnderlyingValue() == 0;
		}
		[[nodiscard]] FORCE_INLINE bool AreAnySet() const
		{
			return !IsEmpty();
		}
		[[nodiscard]] FORCE_INLINE bool AreNoneSet() const
		{
			return GetUnderlyingValue() == 0;
		}
		[[nodiscard]] FORCE_INLINE bool IsSet(const EnumFlagsType other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) != 0;
		}
		[[nodiscard]] FORCE_INLINE bool IsNotSet(const EnumFlagsType other) const
		{
			return !IsSet(other);
		}
		[[nodiscard]] FORCE_INLINE bool AreAnySet(const EnumFlagsType other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) != 0;
		}
		[[nodiscard]] FORCE_INLINE bool AreAllSet(const EnumFlagsType other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) == other.GetUnderlyingValue();
		}
		[[nodiscard]] FORCE_INLINE bool AreAnyNotSet(const EnumFlagsType other) const
		{
			return ((GetUnderlyingValue() ^ other.GetUnderlyingValue()) & other.GetUnderlyingValue()) != 0;
		}
		[[nodiscard]] FORCE_INLINE bool AreNoneSet(const EnumFlagsType other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) == 0;
		}
		FORCE_INLINE void Clear()
		{
			m_flags = 0;
		}
		FORCE_INLINE void Clear(const EnumFlagsType other)
		{
			m_flags &= ~other.GetUnderlyingValue();
		}
		[[nodiscard]] FORCE_INLINE EnumFlagsType FetchClear()
		{
			UnderlyingType currentValue = m_flags;
			while (!m_flags.CompareExchangeWeak(currentValue, UnderlyingType(0)))
				;
			return EnumFlagsType{static_cast<EnumType>(currentValue)};
		}
		FORCE_INLINE void Set(const EnumFlagsType other)
		{
			m_flags |= other.GetUnderlyingValue();
		}

		[[nodiscard]] FORCE_INLINE bool operator==(const EnumFlagsType other) const
		{
			return GetUnderlyingValue() == other.GetUnderlyingValue();
		}
		[[nodiscard]] FORCE_INLINE bool operator!=(const EnumFlagsType other) const
		{
			return GetUnderlyingValue() != other.GetUnderlyingValue();
		}

		[[nodiscard]] FORCE_INLINE EnumType GetFlags() const
		{
			return EnumType(GetUnderlyingValue());
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType operator|(const EnumFlagsType other) const
		{
			return EnumFlagsType(EnumType(GetUnderlyingValue() | other.GetUnderlyingValue()));
		}

		FORCE_INLINE void operator|=(const EnumFlagsType other)
		{
			m_flags |= other.GetUnderlyingValue();
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType FetchOr(const EnumFlagsType other)
		{
			return EnumType(m_flags.FetchOr(other.GetUnderlyingValue()));
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType operator&(const EnumFlagsType other) const
		{
			return EnumFlagsType(EnumType(GetUnderlyingValue() & other.GetUnderlyingValue()));
		}

		FORCE_INLINE void operator&=(const EnumFlagsType other)
		{
			m_flags &= other.GetUnderlyingValue();
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType FetchAnd(const EnumFlagsType other)
		{
			return EnumType(m_flags.FetchAnd(other.GetUnderlyingValue()));
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType operator^(const EnumFlagsType other) const
		{
			return EnumFlagsType(EnumType(GetUnderlyingValue() ^ other.GetUnderlyingValue()));
		}

		FORCE_INLINE void operator^=(const EnumFlagsType other)
		{
			m_flags ^= other.GetUnderlyingValue();
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType FetchXor(const EnumFlagsType other)
		{
			return EnumType(m_flags.FetchXor(other.GetUnderlyingValue()));
		}

		void ClearFlags(const EnumFlagsType other)
		{
			m_flags &= ~other.GetUnderlyingValue();
		}

		FORCE_INLINE bool TryClearFlags(const EnumFlagsType other)
		{
			return FetchAnd(~other).AreAllSet(other);
		}

		FORCE_INLINE bool TrySetFlags(const EnumFlagsType other)
		{
			return !FetchOr(other).AreAllSet(other);
		}

		FORCE_INLINE bool CompareExchangeStrong(EnumFlagsType& expected, const EnumFlagsType newValue)
		{
			return m_flags.CompareExchangeStrong(expected.GetUnderlyingValue(), newValue.GetUnderlyingValue());
		}
		FORCE_INLINE bool CompareExchangeStrong(EnumType& expected, const EnumFlagsType newValue)
		{
			return m_flags.CompareExchangeStrong(reinterpret_cast<UnderlyingType&>(expected), newValue.GetUnderlyingValue());
		}
		FORCE_INLINE bool CompareExchangeWeak(EnumFlagsType& expected, const EnumFlagsType newValue)
		{
			return m_flags.CompareExchangeWeak(expected.GetUnderlyingValue(), newValue.GetUnderlyingValue());
		}
		FORCE_INLINE bool CompareExchangeWeak(EnumType& expected, const EnumFlagsType newValue)
		{
			return m_flags.CompareExchangeWeak(reinterpret_cast<UnderlyingType&>(expected), newValue.GetUnderlyingValue());
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType operator<<(const UnderlyingType shiftValue) const
		{
			return AtomicEnumFlags(GetUnderlyingValue() << shiftValue);
		}
		[[nodiscard]] FORCE_INLINE EnumFlagsType operator>>(const UnderlyingType shiftValue) const
		{
			return AtomicEnumFlags(GetUnderlyingValue() << shiftValue);
		}

		[[nodiscard]] FORCE_INLINE EnumFlagsType operator~() const
		{
			return EnumFlagsType(~GetUnderlyingValue());
		}

		[[nodiscard]] FORCE_INLINE UnderlyingType GetUnderlyingValue() const
		{
			return m_flags;
		}
	protected:
		Threading::Atomic<UnderlyingType> m_flags = 0;
	};
}

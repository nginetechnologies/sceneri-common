#pragma once

#include <Common/Platform/CompilerWarnings.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Guid.h>
#include <Common/TypeTraits/UnderlyingType.h>
#include <Common/Memory/CountBits.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine
{
	namespace Internal
	{
		template<typename UnderlyingType>
		struct TRIVIAL_ABI EnumFlagsBase
		{
			bool Serialize(const Serialization::Reader serializer);
			bool Serialize(Serialization::Writer serializer) const;
		};
	};

	template<typename EnumType>
	struct TRIVIAL_ABI EnumFlags : public Internal::EnumFlagsBase<TypeTraits::UnderlyingType<EnumType>>
	{
		inline static constexpr Guid TypeGuid = "{A0AF848D-ABDE-4B72-94DC-2BE47332B94F}"_guid;

		using UnderlyingType = TypeTraits::UnderlyingType<EnumType>;

		FORCE_INLINE NO_DEBUG constexpr EnumFlags(const EnumType value)
			: m_flags(value)
		{
		}
		FORCE_INLINE NO_DEBUG constexpr EnumFlags() = default;
		FORCE_INLINE NO_DEBUG constexpr EnumFlags(const EnumFlags& other)
			: m_flags(other.m_flags)
		{
		}
		FORCE_INLINE NO_DEBUG constexpr EnumFlags& operator=(const EnumFlags& other)
		{
			m_flags = other.m_flags;
			return *this;
		}
		FORCE_INLINE NO_DEBUG constexpr EnumFlags(EnumFlags&& other)
			: m_flags(other.m_flags)
		{
			other.Clear();
		}
		FORCE_INLINE NO_DEBUG constexpr EnumFlags& operator=(EnumFlags&& other)
		{
			m_flags = other.m_flags;
			other.Clear();
			return *this;
		}
	private:
		FORCE_INLINE NO_DEBUG constexpr explicit EnumFlags(const UnderlyingType value)
			: m_flags(static_cast<EnumType>(value))
		{
		}

		using UnderlyingSetBitsIterator = Memory::SetBitsIterator<UnderlyingType>;
	public:
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool IsEmpty() const
		{
			return GetUnderlyingValue() == 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool AreNoneSet() const
		{
			return GetUnderlyingValue() == 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool AreAnySet() const
		{
			return GetUnderlyingValue() != 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool IsSet(const EnumFlags other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) != 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool IsNotSet(const EnumFlags other) const
		{
			return !IsSet(other);
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool AreNoneSet(const EnumFlags other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) == 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool AreAnySet(const EnumFlags other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) != 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool AreAnyNotSet(const EnumFlags other) const
		{
			return ((GetUnderlyingValue() ^ other.GetUnderlyingValue()) & other.GetUnderlyingValue()) != 0;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool AreAllSet(const EnumFlags other) const
		{
			return (GetUnderlyingValue() & other.GetUnderlyingValue()) == other.GetUnderlyingValue();
		}
		//! Returns EnumFlags with all set flags specified by ranged
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags GetRange(const EnumType first, const EnumType last) const
		{
			const UnderlyingType maskLength =
				UnderlyingType((1 << (static_cast<UnderlyingType>(last) - static_cast<UnderlyingType>(first) + 1)) - 1);
			const UnderlyingType mask = (maskLength << (static_cast<UnderlyingType>(first) - 1)) & GetUnderlyingValue();
			return EnumFlags(mask);
		}
		FORCE_INLINE NO_DEBUG constexpr void Clear()
		{
			m_flags = EnumType();
		}
		FORCE_INLINE NO_DEBUG constexpr void Clear(const EnumFlags flags)
		{
			m_flags = EnumType(GetUnderlyingValue() & ~flags.GetUnderlyingValue());
		}
		FORCE_INLINE NO_DEBUG constexpr void Set(const EnumFlags flags)
		{
			m_flags = EnumType(GetUnderlyingValue() | flags.GetUnderlyingValue());
		}
		FORCE_INLINE NO_DEBUG constexpr void Set(const EnumFlags flags, const bool condition)
		{
			m_flags = EnumType((GetUnderlyingValue() & ~flags.GetUnderlyingValue()) | (flags * condition).GetUnderlyingValue());
		}
		FORCE_INLINE NO_DEBUG constexpr void Toggle(const EnumFlags flags)
		{
			m_flags = EnumType(GetUnderlyingValue() ^ flags.GetUnderlyingValue());
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator==(const EnumFlags other) const
		{
			return GetUnderlyingValue() == other.GetUnderlyingValue();
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator!=(const EnumFlags other) const
		{
			return GetUnderlyingValue() != other.GetUnderlyingValue();
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumType GetFlags() const
		{
			return m_flags;
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator|(const EnumFlags other) const
		{
			return EnumFlags(GetUnderlyingValue() | other.GetUnderlyingValue());
		}

		FORCE_INLINE NO_DEBUG constexpr EnumFlags& operator|=(const EnumFlags other)
		{
			GetUnderlyingValue() |= other.GetUnderlyingValue();
			return *this;
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator&(const EnumFlags other) const
		{
			return EnumFlags(GetUnderlyingValue() & other.GetUnderlyingValue());
		}

		FORCE_INLINE NO_DEBUG constexpr EnumFlags& operator&=(const EnumFlags other)
		{
			GetUnderlyingValue() &= static_cast<UnderlyingType>(other.GetUnderlyingValue());
			return *this;
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator^(const EnumFlags other) const
		{
			return EnumFlags(GetUnderlyingValue() ^ other.GetUnderlyingValue());
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator*(const bool value) const
		{
			return EnumFlags(GetUnderlyingValue() * value);
		}

		FORCE_INLINE NO_DEBUG constexpr EnumFlags& operator^=(const EnumFlags other)
		{
			GetUnderlyingValue() ^= static_cast<UnderlyingType>(other.GetUnderlyingValue());
			return *this;
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator<<(const UnderlyingType shiftValue) const
		{
			return EnumFlags(GetUnderlyingValue() << shiftValue);
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator>>(const UnderlyingType shiftValue) const
		{
			return EnumFlags(GetUnderlyingValue() << shiftValue);
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr EnumFlags operator~() const
		{
			return EnumFlags(~GetUnderlyingValue());
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG UnderlyingType& GetUnderlyingValue()
		{
			return reinterpret_cast<UnderlyingType&>(m_flags);
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr UnderlyingType GetUnderlyingValue() const
		{
			return static_cast<UnderlyingType>(m_flags);
		}

		struct Iterator : private UnderlyingSetBitsIterator::Iterator
		{
			using BaseType = typename UnderlyingSetBitsIterator::Iterator;
			Iterator(const BaseType& iterator)
				: BaseType(iterator)
			{
			}
			using BaseType::BaseType;

			[[nodiscard]] FORCE_INLINE NO_DEBUG bool operator==(const Iterator& other) const
			{
				return BaseType::operator==(other);
			}

			[[nodiscard]] FORCE_INLINE NO_DEBUG bool operator!=(const Iterator& other) const
			{
				return BaseType::operator!=(other);
			}
			using BaseType::operator++;
			using BaseType::IsSet;

			[[nodiscard]] FORCE_INLINE NO_DEBUG EnumType operator*() const
			{
				return static_cast<EnumType>(1 << BaseType::operator*());
			}
		};

		[[nodiscard]] FORCE_INLINE NO_DEBUG Iterator begin() const
		{
			return Iterator(UnderlyingSetBitsIterator(GetUnderlyingValue()).begin());
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG Iterator end() const
		{
			return Iterator(UnderlyingSetBitsIterator(GetUnderlyingValue()).end());
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG Optional<EnumType> GetFirstSetFlag() const
		{
			const Memory::BitIndex<UnderlyingType> firstSetIndex = Memory::GetFirstSetIndex(GetUnderlyingValue());
			return Optional<EnumType>{static_cast<EnumType>(1 << *firstSetIndex), firstSetIndex.IsValid()};
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG UnderlyingType GetNumberOfSetFlags() const
		{
			return Memory::GetNumberOfSetBits(GetUnderlyingValue());
		}
	protected:
		EnumType m_flags = static_cast<EnumType>(0);
	};
}

#pragma once

#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Assert/Assert.h>
#include <Common/Math/Max.h>
#include <Common/Memory/NativeCharType.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/TypeTraits/IsSame.h>

#include "ForwardDeclarations/ZeroTerminatedStringView.h"

namespace ngine
{
	template<typename InternalCharType, typename InternalSizeType>
	struct TRIVIAL_ABI TZeroTerminatedStringView
	{
		using SizeType = InternalSizeType;
		using CharType = InternalCharType;
		using ViewType = TStringView<CharType, SizeType>;
		using ConstViewType = TStringView<const CharType, SizeType>;

		constexpr TZeroTerminatedStringView() = default;
		constexpr TZeroTerminatedStringView(CharType* pData, const SizeType size)
			: m_pData(pData)
			, m_size(size)
		{
			Assert(IsZeroTerminated());
		}
		template<size Size>
		constexpr TZeroTerminatedStringView& operator=(const CharType (&data)[Size] LIFETIME_BOUND)
		{
			m_pData = data;
			m_size = Size;
			Assert(IsZeroTerminated());
			return *this;
		}
		template<size Size>
		constexpr TZeroTerminatedStringView(CharType (&data)[Size] LIFETIME_BOUND)
			: TZeroTerminatedStringView(data, Size)
		{
		}
		template<typename OtherSizeType>
		constexpr TZeroTerminatedStringView(const TZeroTerminatedStringView<CharType, OtherSizeType>& other)
			: m_pData(other.m_pData)
			, m_size(static_cast<SizeType>(other.m_size))
		{
			Assert(IsZeroTerminated());
		}
		template<typename OtherSizeType>
		constexpr TZeroTerminatedStringView& operator=(const TZeroTerminatedStringView<CharType, OtherSizeType>& other)
		{
			m_pData = other.m_pData;
			m_size = static_cast<SizeType>(other.m_size);
			Assert(IsZeroTerminated());
			return *this;
		}
		template<typename OtherSizeType>
		constexpr TZeroTerminatedStringView(TZeroTerminatedStringView<CharType, OtherSizeType>&& other)
			: m_pData(other.GetData())
			, m_size(static_cast<SizeType>(other.m_size))
		{
			Assert(IsZeroTerminated());
		}
		template<typename OtherSizeType>
		constexpr TZeroTerminatedStringView& operator=(TZeroTerminatedStringView<CharType, OtherSizeType>&& other)
		{
			m_pData = other.m_pData;
			m_size = static_cast<SizeType>(other.m_size);
			other.m_pData = nullptr;
			other.m_size = nullptr;
			Assert(IsZeroTerminated());
			return *this;
		}

		[[nodiscard]] FORCE_INLINE operator CharType*() const
		{
			return m_pData;
		}
		[[nodiscard]] FORCE_INLINE CharType* GetData() const
		{
			return m_pData;
		}

		[[nodiscard]] FORCE_INLINE operator ConstViewType() const
		{
			return ViewType(m_pData, GetSize());
		}
		[[nodiscard]] FORCE_INLINE operator ViewType()
		{
			return ViewType(m_pData, GetSize());
		}
		[[nodiscard]] FORCE_INLINE ConstViewType GetView() const
		{
			return ViewType(m_pData, GetSize());
		}
		[[nodiscard]] FORCE_INLINE ViewType GetView()
		{
			return ViewType(m_pData, GetSize());
		}

		[[nodiscard]] FORCE_INLINE SizeType GetSize() const
		{
			return Math::Max(m_size, (SizeType)1u) - 1u;
		}

		[[nodiscard]] FORCE_INLINE bool HasElements() const
		{
			return m_size > 1;
		}
		[[nodiscard]] FORCE_INLINE bool IsEmpty() const
		{
			return !HasElements();
		}

		[[nodiscard]] bool operator==(const TZeroTerminatedStringView other) const
		{
			return ViewType(*this) == ViewType(other);
		}
		[[nodiscard]] bool operator!=(const TZeroTerminatedStringView other) const
		{
			return ViewType(*this) != ViewType(other);
		}
		[[nodiscard]] bool operator==(const ConstViewType other) const
		{
			return ViewType(*this) == other;
		}
		[[nodiscard]] bool operator!=(const ConstViewType other) const
		{
			return ViewType(*this) != other;
		}

#if PLATFORM_WINDOWS
		template<
			typename CharType = InternalCharType,
			typename = EnableIf<TypeTraits::IsSame<CharType, const UnicodeCharType> && TypeTraits::IsConst<CharType>>>
		[[nodiscard]] FORCE_INLINE operator const NativeCharType *() const LIFETIME_BOUND
		{
			return reinterpret_cast<const NativeCharType*>(GetData());
		}

		template<
			typename CharType = InternalCharType,
			typename = EnableIf<TypeTraits::IsSame<CharType, UnicodeCharType> && !TypeTraits::IsConst<CharType>>>
		[[nodiscard]] FORCE_INLINE operator NativeCharType*() const LIFETIME_BOUND
		{
			return reinterpret_cast<NativeCharType*>(GetData());
		}
#endif
	protected:
		[[nodiscard]] FORCE_INLINE constexpr bool IsZeroTerminated() const
		{
			return m_pData == nullptr || *(m_pData + m_size - 1u) == '\0';
		}

		template<typename OtherCharType, typename OtherSizeType>
		friend struct TZeroTerminatedStringView;

		CharType* m_pData = nullptr;
		SizeType m_size = 0u;
	};
}

#pragma once

#include "IdentifierArrayView.h"
#include <Common/Memory/Containers/FixedArrayView.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Storage/ForwardDeclarations/FixedIdentifierArrayView.h>

namespace ngine
{
	template<typename ContainedType, typename IdentifierType, uint8 Flags>
	struct TRIVIAL_ABI FixedIdentifierArrayView : protected FixedArrayView<
																									ContainedType,
																									IdentifierType::MaximumCount,
																									typename IdentifierType::IndexType,
																									typename IdentifierType::IndexType,
																									Flags>
	{
		using BaseType = FixedArrayView<
			ContainedType,
			IdentifierType::MaximumCount,
			typename IdentifierType::IndexType,
			typename IdentifierType::IndexType,
			Flags>;
		using View = FixedIdentifierArrayView<ContainedType, IdentifierType, Flags>;
		using ConstView = FixedIdentifierArrayView<ContainedType, IdentifierType, Flags>;
		using DynamicView = IdentifierArrayView<ContainedType, IdentifierType, Flags>;
		using ConstDynamicView = IdentifierArrayView<const ContainedType, IdentifierType, Flags>;
		using SizeType = typename BaseType::SizeType;
		using RestrictedView = FixedIdentifierArrayView<ContainedType, IdentifierType, Flags | (uint8)ArrayViewFlags::Restrict>;
		using StoredType = typename BaseType::StoredType;

		using BaseType::BaseType;
		using BaseType::operator=;
		FixedIdentifierArrayView(const BaseType& other)
			: BaseType(other)
		{
		}
		FixedIdentifierArrayView(BaseType&& other)
			: BaseType(Forward<BaseType>(other))
		{
		}

		[[nodiscard]] FORCE_INLINE operator RestrictedView() const
		{
			return RestrictedView{BaseType::m_data};
		}

		[[nodiscard]] FORCE_INLINE ContainedType& operator[](const IdentifierType identifier)
		{
			return BaseType::operator[](identifier.GetFirstValidIndex());
		}
		[[nodiscard]] FORCE_INLINE const ContainedType& operator[](const IdentifierType identifier) const
		{
			return BaseType::operator[](identifier.GetFirstValidIndex());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewUpTo(const SizeType index) const noexcept
		{
			return DynamicView(*this).GetSubViewUpTo(begin() + index);
		}

		using BaseType::IsValidIndex;
		using BaseType::GetData;
		using BaseType::GetDataSize;
		using BaseType::GetSize;
		using BaseType::begin;
		using BaseType::end;
		using BaseType::DefaultConstruct;
		using BaseType::ZeroInitialize;
		using BaseType::GetIteratorIndex;
		using BaseType::IsWithinBounds;
		using BaseType::DestroyElements;

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		CopyConstruct(const ConstView other) const noexcept
		{
			BaseType::View(*this).CopyConstruct(other);
		}
		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		CopyConstruct(const ConstDynamicView other) const noexcept
		{
			BaseType::DynamicView(*this).CopyConstruct(other);
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		MoveConstruct(const View other) const noexcept
		{
			BaseType::DynamicView(*this).MoveConstruct(other);
		}
		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		MoveConstruct(const DynamicView other) const noexcept
		{
			BaseType::DynamicView(*this).MoveConstruct(other);
		}
	};
}

#pragma once

#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Storage/ForwardDeclarations/IdentifierArrayView.h>

namespace ngine
{
	template<typename ContainedType, typename IdentifierType, uint8 Flags>
	struct TRIVIAL_ABI IdentifierArrayView
		: public ArrayView<ContainedType, typename IdentifierType::IndexType, typename IdentifierType::IndexType, ContainedType, Flags>
	{
		using BaseType = ArrayView<ContainedType, typename IdentifierType::IndexType, typename IdentifierType::IndexType, ContainedType, Flags>;
		using RestrictedView = IdentifierArrayView<ContainedType, IdentifierType, Flags | (uint8)ArrayViewFlags::Restrict>;

		using BaseType::BaseType;
		using BaseType::operator=;
		using typename BaseType::StoredType;
		using BaseType::operator++;
		using BaseType::operator[];
		using BaseType::HasElements;

		IdentifierArrayView(const BaseType& other)
			: BaseType(other)
		{
		}
		IdentifierArrayView(BaseType&& other)
			: BaseType(Forward<BaseType>(other))
		{
		}

		[[nodiscard]] FORCE_INLINE operator RestrictedView() const
		{
			return *this;
		}

		[[nodiscard]] FORCE_INLINE ContainedType& operator[](const IdentifierType identifier)
		{
			return BaseType::operator[](identifier.GetFirstValidIndex());
		}
		[[nodiscard]] FORCE_INLINE const ContainedType& operator[](const IdentifierType identifier) const
		{
			return BaseType::operator[](identifier.GetFirstValidIndex());
		}

		using BaseType::IsValidIndex;
		using BaseType::GetData;
		using BaseType::GetDataSize;
		using BaseType::GetSize;
		using BaseType::GetSubViewUpTo;
		using BaseType::begin;
		using BaseType::end;
		using BaseType::ZeroInitialize;
		using BaseType::GetIteratorIndex;
		using BaseType::IsWithinBounds;
	};
}

#pragma once

#include <Common/Memory/Containers/ForwardDeclarations/ArrayView.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<
		typename ContainedType,
		typename InternalSizeType = uint32,
		typename InternalIndexType = InternalSizeType,
		typename StoredType = ContainedType>
	struct TRIVIAL_ABI RestrictedArrayView
		: public ArrayView<ContainedType, InternalSizeType, InternalIndexType, StoredType, (uint8)ArrayViewFlags::Restrict>
	{
		using BaseType = ArrayView<ContainedType, InternalSizeType, InternalIndexType, StoredType, (uint8)ArrayViewFlags::Restrict>;
		using BaseType::BaseType;

		using SizeType = typename BaseType::SizeType;
		using IndexType = typename BaseType::IndexType;

		template<typename OtherSizeType = SizeType, typename OtherIndexType = IndexType>
		constexpr RestrictedArrayView(const ArrayView<ContainedType, OtherSizeType, OtherIndexType, StoredType, 0>& otherView) noexcept
			: BaseType(otherView.begin(), static_cast<SizeType>(otherView.GetSize()))
		{
		}
		template<typename OtherSizeType = SizeType, typename OtherIndexType = IndexType>
		constexpr RestrictedArrayView& operator=(const ArrayView<ContainedType, OtherSizeType, OtherIndexType, StoredType, 0> otherView
		) noexcept
		{
			BaseType::operator=(otherView.m_pBegin, static_cast<SizeType>(otherView.GetSize()));
			return *this;
		}

		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		constexpr RestrictedArrayView(
			const ArrayView<typename TypeTraits::WithoutConst<ElementType>, OtherSizeType, OtherIndexType, StoredType, 0>& otherView
		) noexcept
			: BaseType(otherView.begin(), (SizeType)otherView.GetSize())
		{
		}
		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		constexpr RestrictedArrayView&
		operator=(const ArrayView<typename TypeTraits::WithoutConst<ElementType>, OtherSizeType, OtherIndexType, StoredType, 0> otherView
		) noexcept
		{
			BaseType::operator=(otherView.m_pBegin, otherView.m_size);
			return *this;
		}
	};
}

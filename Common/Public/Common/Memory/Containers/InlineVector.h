#pragma once

#include <Common/Memory/Allocators/DynamicInlineStorageAllocator.h>
#include "VectorBase.h"
#include "ForwardDeclarations/InlineVector.h"

namespace ngine
{
	template<typename ContainedType, size InlineCapacity, typename SizeType, typename IndexType>
	struct InlineVector : public TVector<
													ContainedType,
													Memory::DynamicInlineStorageAllocator<ContainedType, InlineCapacity, SizeType, IndexType>,
													Memory::VectorFlags::AllowResize | Memory::VectorFlags::AllowReallocate>
	{
		using BaseType = TVector<
			ContainedType,
			Memory::DynamicInlineStorageAllocator<ContainedType, InlineCapacity, SizeType, IndexType>,
			Memory::VectorFlags::AllowResize | Memory::VectorFlags::AllowReallocate>;
		using BaseType::BaseType;
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		InlineVector(InlineVector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		// template <typename ElementType = ContainedType>
		explicit InlineVector(const InlineVector& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		InlineVector& operator=(InlineVector&& other) noexcept
		{
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		InlineVector& operator=(const InlineVector& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		using BaseType::operator=;
	};

	template<typename ContainedType, size InlineCapacity, typename SizeType, typename IndexType>
	struct FixedSizeInlineVector : public TVector<
																	 ContainedType,
																	 Memory::DynamicInlineStorageAllocator<ContainedType, InlineCapacity, SizeType, IndexType>,
																	 Memory::VectorFlags::None>
	{
		using BaseType = TVector<
			ContainedType,
			Memory::DynamicInlineStorageAllocator<ContainedType, InlineCapacity, SizeType, IndexType>,
			Memory::VectorFlags::None>;
		using BaseType::BaseType;

		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedSizeInlineVector(FixedSizeInlineVector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		explicit FixedSizeInlineVector(const FixedSizeInlineVector& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedSizeInlineVector& operator=(FixedSizeInlineVector&& other) noexcept
		{
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		FixedSizeInlineVector& operator=(const FixedSizeInlineVector& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		using BaseType::operator=;
	};

	template<typename ContainedType, size InlineCapacity, typename SizeType, typename IndexType>
	struct FixedCapacityInlineVector : public TVector<
																			 ContainedType,
																			 Memory::DynamicInlineStorageAllocator<ContainedType, InlineCapacity, SizeType, IndexType>,
																			 Memory::VectorFlags::AllowResize>
	{
		using BaseType = TVector<
			ContainedType,
			Memory::DynamicInlineStorageAllocator<ContainedType, InlineCapacity, SizeType, IndexType>,
			Memory::VectorFlags::AllowResize>;
		using BaseType::BaseType;

		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedCapacityInlineVector(FixedCapacityInlineVector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		explicit FixedCapacityInlineVector(const FixedCapacityInlineVector& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedCapacityInlineVector& operator=(FixedCapacityInlineVector&& other) noexcept
		{
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		FixedCapacityInlineVector& operator=(const FixedCapacityInlineVector& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		using BaseType::operator=;
	};
}

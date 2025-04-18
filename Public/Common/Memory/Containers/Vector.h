#pragma once

#include "VectorBase.h"

#include "ForwardDeclarations/Vector.h"
#include <Common/Memory/Allocators/DynamicAllocator.h>

#include <Common/TypeTraits/EnforceConvertibleTo.h>
#include <Common/Memory/GetNumericSize.h>

namespace ngine
{
	template<typename ContainedType, typename SizeType, typename IndexType, typename AllocatorType>
	struct Vector : public TVector<ContainedType, AllocatorType, Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>
	{
		using BaseType = TVector<ContainedType, AllocatorType, Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
		using BaseType::BaseType;

		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		Vector(Vector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		// template <typename ElementType = ContainedType>
		explicit Vector(const Vector& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		Vector& operator=(Vector&& other) noexcept
		{
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		Vector& operator=(const Vector& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		using BaseType::operator=;
	};

	template<class FirstType, class... RemainingTypes>
	Vector(FirstType, RemainingTypes...) -> Vector<
		typename TypeTraits::EnforceConvertibleTo<FirstType, RemainingTypes...>::Type,
		Memory::NumericSize<1 + sizeof...(RemainingTypes)>>;

	template<typename ContainedType, typename SizeType, typename IndexType, typename AllocatorType>
	struct FixedCapacityVector : public TVector<ContainedType, AllocatorType, Memory::VectorFlags::AllowResize>
	{
		using BaseType = TVector<ContainedType, AllocatorType, Memory::VectorFlags::AllowResize>;
		using BaseType::BaseType;

		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedCapacityVector(FixedCapacityVector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		explicit FixedCapacityVector(const FixedCapacityVector& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedCapacityVector& operator=(FixedCapacityVector&& other) noexcept
		{
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		FixedCapacityVector& operator=(const FixedCapacityVector& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		using BaseType::operator=;
	};

	template<class FirstType, class... RemainingTypes>
	FixedCapacityVector(FirstType, RemainingTypes...) -> FixedCapacityVector<
		typename TypeTraits::EnforceConvertibleTo<FirstType, RemainingTypes...>::Type,
		Memory::NumericSize<1 + sizeof...(RemainingTypes)>>;

	template<typename ContainedType, typename SizeType, typename IndexType, typename AllocatorType>
	struct FixedSizeVector : public TVector<ContainedType, AllocatorType, Memory::VectorFlags::None>
	{
		using BaseType = TVector<ContainedType, AllocatorType, Memory::VectorFlags::None>;
		using BaseType::BaseType;

		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedSizeVector(FixedSizeVector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		explicit FixedSizeVector(const FixedSizeVector& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		FixedSizeVector& operator=(FixedSizeVector&& other) noexcept
		{
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		FixedSizeVector& operator=(const FixedSizeVector& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		using BaseType::operator=;
	};

	template<class FirstType, class... RemainingTypes>
	FixedSizeVector(FirstType, RemainingTypes...) -> FixedSizeVector<
		typename TypeTraits::EnforceConvertibleTo<FirstType, RemainingTypes...>::Type,
		Memory::NumericSize<1 + sizeof...(RemainingTypes)>>;
}

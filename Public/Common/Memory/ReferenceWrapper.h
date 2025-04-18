#pragma once

#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/WithoutConst.h>
#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/HasMemberVariable.h>
#include <Common/TypeTraits/IsPointerComparable.h>
#include <Common/TypeTraits/IsEqualityComparable.h>
#include <Common/Guid.h>

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Assume.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/Assume.h>
#include <Common/Platform/NoDebug.h>

namespace ngine
{
	template<typename ElementType>
	struct TRIVIAL_ABI ReferenceWrapper
	{
		HasMemberVariable(ElementType, TypeGuid);
		inline static constexpr auto TypeGuid = []()
		{
			if constexpr (HasTypeGuid)
				return ElementType::TypeGuid;
			else
				return Guid();
		}();

		ReferenceWrapper() = delete;

		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper(ElementType& element) noexcept
			: m_pElement(&element)
		{
		}
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper(const ReferenceWrapper& other) noexcept
			: m_pElement(other.m_pElement)
		{
		}
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper& operator=(const ReferenceWrapper& other) noexcept
		{
			m_pElement = other.m_pElement;
			return *this;
		}
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper(ReferenceWrapper&& other) noexcept
			: m_pElement(other.m_pElement)
		{
		}
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper& operator=(ReferenceWrapper&& other) noexcept
		{
			m_pElement = other.m_pElement;
			return *this;
		}
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper& operator=(ElementType& element) noexcept
		{
			m_pElement = &element;
			return *this;
		}
		template<typename ContainedType = ElementType, typename = EnableIf<TypeTraits::IsConst<ContainedType>>>
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper(const ReferenceWrapper<TypeTraits::WithoutConst<ContainedType>>& other) noexcept
			: m_pElement(&*other)
		{
		}
		template<typename ContainedType = ElementType, typename = EnableIf<TypeTraits::IsConst<ContainedType>>>
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper& operator=(const ReferenceWrapper<TypeTraits::WithoutConst<ContainedType>>& other
		) noexcept
		{
			m_pElement = &*other;
			return *this;
		}
		template<
			typename OtherType,
			typename ThisType = ElementType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ThisType, OtherType> &&
				!TypeTraits::IsSame<TypeTraits::WithoutConst<ThisType>, TypeTraits::WithoutConst<OtherType>>>>
		FORCE_INLINE NO_DEBUG constexpr ReferenceWrapper(const ReferenceWrapper<OtherType>& other)
			: m_pElement(&other)
		{
		}
		template<
			typename OtherType,
			typename ThisType = ElementType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ThisType, OtherType> &&
				!TypeTraits::IsSame<TypeTraits::WithoutConst<ThisType>, TypeTraits::WithoutConst<OtherType>>>>
		FORCE_INLINE NO_DEBUG ReferenceWrapper constexpr operator=(const ReferenceWrapper<OtherType>& other)
		{
			m_pElement = &other;
			return *this;
		}
		~ReferenceWrapper() = default;

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr operator ElementType&() const noexcept
		{
			ASSUME(m_pElement != nullptr);
			return *m_pElement;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr ElementType* operator->() const noexcept
		{
			ASSUME(m_pElement != nullptr);
			return m_pElement;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr ElementType& operator*() const noexcept
		{
			ASSUME(m_pElement != nullptr);
			return *m_pElement;
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr ElementType* operator&() const noexcept
		{
			ASSUME(m_pElement != nullptr);
			return m_pElement;
		}

		template<
			typename OtherElementType = ElementType,
			typename ThisElementType = ElementType,
			typename = EnableIf<TypeTraits::IsPointerComparable<OtherElementType, ThisElementType>>>
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator==(const OtherElementType& other) const noexcept
		{
			if (m_pElement == Memory::GetAddressOf(other))
			{
				return true;
			}

			if constexpr (TypeTraits::IsEqualityComparable<OtherElementType, ThisElementType>)
			{
				return *m_pElement == other;
			}
			else
			{
				return false;
			}
		}
		template<
			typename OtherElementType = ElementType,
			typename ThisElementType = ElementType,
			typename = EnableIf<TypeTraits::IsPointerComparable<OtherElementType, ThisElementType>>>
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator!=(const OtherElementType& other) const noexcept
		{
			return !operator==(other);
		}

		template<typename ThisElementType = ElementType, typename = EnableIf<TypeTraits::IsConst<ThisElementType>>>
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator==(const ReferenceWrapper<TypeTraits::WithoutConst<ElementType>> other
		) const noexcept
		{
			return operator==(*other);
		}

		template<typename ThisElementType = ElementType, typename = EnableIf<!TypeTraits::IsConst<ThisElementType>>>
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator==(const ReferenceWrapper<const ElementType> other) const noexcept
		{
			return operator==(*other);
		}

		template<typename ThisElementType = ElementType, typename = EnableIf<TypeTraits::IsConst<ThisElementType>>>
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator!=(const ReferenceWrapper<TypeTraits::WithoutConst<ElementType>> other
		) const noexcept
		{
			return !operator==(*other);
		}

		template<typename ThisElementType = ElementType, typename = EnableIf<!TypeTraits::IsConst<ThisElementType>>>
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator!=(const ReferenceWrapper<const ElementType> other) const noexcept
		{
			return !operator==(*other);
		}

		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator==(const ReferenceWrapper other) const noexcept
		{
			return operator==(*other);
		}
		[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr bool operator!=(const ReferenceWrapper other) const noexcept
		{
			return !operator==(*other);
		}
	protected:
		ElementType* m_pElement;
	};

	template<typename ElementType>
	[[nodiscard]] FORCE_INLINE NO_DEBUG bool operator==(ElementType& element, const ReferenceWrapper<ElementType> wrapper) noexcept
	{
		return wrapper == element;
	}

	template<typename ElementType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
	[[nodiscard]] FORCE_INLINE NO_DEBUG bool operator==(const ElementType& element, const ReferenceWrapper<ElementType> wrapper) noexcept
	{
		return wrapper == element;
	}
}

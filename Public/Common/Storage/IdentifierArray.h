#pragma once

#include <Common/Storage/Identifier.h>
#include <Common/Storage/FixedIdentifierArrayView.h>
#include <Common/Storage/IdentifierArrayView.h>

#include <Common/Memory/AddressOf.h>
#include <Common/Memory/Allocators/FixedAllocator.h>

namespace ngine
{
	template<typename ContainedType, typename IdentifierType>
	struct TIdentifierArray
	{
		using AllocatorType = Memory::
			FixedAllocator<ContainedType, IdentifierType::MaximumCount, typename IdentifierType::IndexType, typename IdentifierType::IndexType>;
		using SizeType = typename AllocatorType::SizeType;
		using View = FixedIdentifierArrayView<ContainedType, IdentifierType>;
		using ConstView = FixedIdentifierArrayView<const ContainedType, IdentifierType>;
		using RestrictedView = FixedIdentifierArrayView<ContainedType, IdentifierType, (uint8)ArrayViewFlags::Restrict>;
		using ConstRestrictedView = FixedIdentifierArrayView<const ContainedType, IdentifierType, (uint8)ArrayViewFlags::Restrict>;
		using DynamicView = IdentifierArrayView<ContainedType, IdentifierType>;
		using ConstDynamicView = IdentifierArrayView<const ContainedType, IdentifierType>;

		TIdentifierArray()
		{
			m_allocator.GetView().DefaultConstruct();
		}

		template<typename... ConstructArgs>
		TIdentifierArray(const Memory::InitializeAllType, ConstructArgs&&... args)
		{
			m_allocator.GetView().InitializeAll(Forward<ConstructArgs>(args)...);
		}

		TIdentifierArray(const Memory::DefaultConstructType)
		{
			m_allocator.GetView().DefaultConstruct();
		}

		TIdentifierArray(const Memory::ZeroedType)
		{
			m_allocator.GetView().ZeroInitialize();
		}

		TIdentifierArray(const Memory::UninitializedType)
		{
		}

		TIdentifierArray(const TIdentifierArray&) = delete;
		TIdentifierArray& operator=(const TIdentifierArray&) = delete;

		template<
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsMoveConstructible<ElementType>>>
		TIdentifierArray(TIdentifierArray&& other)
		{
			GetView().MoveConstruct(other.GetView());
		}
		template<
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsMoveConstructible<ElementType>>>
		TIdentifierArray& operator=(TIdentifierArray&& other)
		{
			m_allocator.GetView().MoveConstruct(other.GetView());
			return *this;
		}

		~TIdentifierArray()
		{
			DestroyAll();
		}

		template<typename... ConstructArgs>
		void Construct(const IdentifierType identifier, ConstructArgs&&... args)
		{
			ContainedType& element = m_allocator.GetView()[identifier.GetFirstValidIndex()];
			new (Memory::GetAddressOf(element)) ContainedType(Forward<ConstructArgs>(args)...);
		}

		void Destroy(const IdentifierType identifier)
		{
			ContainedType& element = m_allocator.GetView()[identifier.GetFirstValidIndex()];
			element.~ContainedType();
		}

		void DestroyAll()
		{
			m_allocator.GetView().DestroyElements();
		}

		[[nodiscard]] FORCE_INLINE typename IdentifierType::IndexType GetElementIndex(const ContainedType& containedElement) const
		{
			return GetView().GetIteratorIndex(Memory::GetAddressOf(containedElement));
		}

		[[nodiscard]] FORCE_INLINE View GetView()
		{
			return m_allocator.GetView();
		}
		[[nodiscard]] FORCE_INLINE ConstView GetView() const
		{
			return m_allocator.GetView();
		}
		[[nodiscard]] FORCE_INLINE operator View()
		{
			return m_allocator.GetView();
		}
		[[nodiscard]] FORCE_INLINE operator ConstView() const
		{
			return m_allocator.GetView();
		}
		[[nodiscard]] FORCE_INLINE DynamicView GetDynamicView()
		{
			return m_allocator.GetView().GetDynamicView();
		}
		[[nodiscard]] FORCE_INLINE ConstDynamicView GetDynamicView() const
		{
			return m_allocator.GetView().GetDynamicView();
		}

		[[nodiscard]] FORCE_INLINE ContainedType& operator[](const IdentifierType identifier)
		{
			return m_allocator.GetView()[identifier.GetFirstValidIndex()];
		}
		[[nodiscard]] FORCE_INLINE const ContainedType& operator[](const IdentifierType identifier) const
		{
			return m_allocator.GetView()[identifier.GetFirstValidIndex()];
		}
	protected:
		AllocatorType m_allocator;
	};
}

#pragma once

#include <Common/Assert/Assert.h>
#include "Forward.h"
#include "Move.h"
#include "UniquePtrView.h"
#include <Common/Memory/Optional.h>

#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Memory/Invalid.h>
#include <Common/Memory/Containers/ContainerCommon.h>

#include <Common/Memory/ForwardDeclarations/UniqueRef.h>

namespace ngine
{
	template<typename ContainedType>
	struct UniqueRefNullable;

	template<typename ContainedType>
	struct TRIVIAL_ABI UniquePtr
	{
		using View = UniquePtrView<ContainedType>;
		using ConstView = UniquePtrView<const ContainedType>;

		constexpr UniquePtr() = default;
		constexpr UniquePtr(decltype(nullptr)) noexcept
		{
		}
		constexpr UniquePtr(InvalidType) noexcept
		{
		}
		UniquePtr& operator=(InvalidType) noexcept
		{
			DestroyElement();
			return *this;
		}

		template<typename... Args>
		constexpr UniquePtr(Memory::ConstructInPlaceType, Args&&... args)
			: m_pElement(new ContainedType{Forward<Args>(args)...})
		{
		}

		UniquePtr(UniqueRef<ContainedType>&& uniqueRef) = delete;
		UniquePtr& operator=(UniqueRef<ContainedType>&&) = delete;
		explicit UniquePtr(ContainedType* pElement) noexcept
			: m_pElement(pElement)
		{
			Assert(pElement != nullptr);
		}

		UniquePtr(const UniquePtr&) = delete;
		UniquePtr& operator=(const UniquePtr&) = delete;

		UniquePtr(UniquePtr&& other) noexcept
			: m_pElement(other.ReleaseOwnership())
		{
		}

		UniquePtr& operator=(UniquePtr&& other) noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
			}
			m_pElement = other.ReleaseOwnership();
			return *this;
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		UniquePtr(UniquePtr<DerivedType>&& other) noexcept
			: m_pElement(static_cast<ContainedType*>(other.ReleaseOwnership()))
		{
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		UniquePtr& operator=(UniquePtr<DerivedType>&& other) noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
			}
			m_pElement = static_cast<ContainedType*>(other.ReleaseOwnership());
			return *this;
		}

		~UniquePtr() noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
				m_pElement = nullptr;
			}
		}

		template<class... Args>
		[[nodiscard]] static UniquePtr Make(Args&&... args) noexcept
		{
			return UniquePtr(new ContainedType(Forward<Args>(args)...));
		}

		[[nodiscard]] PURE_STATICS static UniquePtr FromRaw(ContainedType* pElement) noexcept
		{
			return UniquePtr(pElement);
		}
		[[nodiscard]] PURE_STATICS static UniquePtr FromRaw(ContainedType& element) noexcept
		{
			return UniquePtr(Memory::GetAddressOf(element));
		}

		[[nodiscard]] PURE_STATICS NO_DEBUG Optional<ContainedType*> Get() const noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG ContainedType& GetReference() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool IsValid() const noexcept
		{
			return m_pElement != nullptr;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool IsInvalid() const noexcept
		{
			return m_pElement == nullptr;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG ContainedType* operator->() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG ContainedType& operator*() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG operator Optional<ContainedType*>() const noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG operator View() noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG operator ConstView() const noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG constexpr operator bool() const noexcept
		{
			return m_pElement != nullptr;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool constexpr operator!=(decltype(nullptr)) const noexcept
		{
			return m_pElement != nullptr;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool constexpr operator==(decltype(nullptr)) const noexcept
		{
			return m_pElement == nullptr;
		}

		template<class... Args>
		ContainedType& CreateInPlace(Args&&... args) noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
			}

			ContainedType* pElement = new ContainedType(Forward<Args>(args)...);
			m_pElement = pElement;
			return *pElement;
		}

		void DestroyElement() noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
				m_pElement = nullptr;
			}
		}

		[[nodiscard]] ContainedType* StealOwnership() noexcept
		{
			Expect(m_pElement != nullptr);
			ContainedType* pElement = m_pElement;
			m_pElement = nullptr;
			return pElement;
		}
	protected:
		template<typename OtherType>
		friend struct UniquePtr;
		template<typename OtherType>
		friend struct UniqueRefNullable;

		[[nodiscard]] ContainedType* ReleaseOwnership() noexcept
		{
			ContainedType* pElement = m_pElement;
			m_pElement = nullptr;
			return pElement;
		}
	protected:
		ContainedType* m_pElement = nullptr;
	};

	//! Unique pointer that cannot be null initialized
	template<typename ContainedType>
	struct TRIVIAL_ABI UniqueRefNullable : public UniquePtr<ContainedType>
	{
		using BaseType = UniquePtr<ContainedType>;

		UniqueRefNullable() = default;

		explicit UniqueRefNullable(ContainedType* pElement) noexcept
			: BaseType(pElement)
		{
			Assert(BaseType::IsValid());
		}

		UniqueRefNullable(BaseType&& other) noexcept
			: BaseType(other.ReleaseOwnership())
		{
			Assert(BaseType::IsValid());
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		UniqueRefNullable(UniquePtr<DerivedType>&& other) noexcept
			: BaseType(static_cast<ContainedType*>(other.ReleaseOwnership()))
		{
			Assert(BaseType::IsValid());
		}
	};
}

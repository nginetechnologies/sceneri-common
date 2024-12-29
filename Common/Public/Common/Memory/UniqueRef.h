#pragma once

#include <Common/Assert/Assert.h>
#include "Forward.h"

#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Memory/Containers/ContainerCommon.h>

#include <Common/Memory/ForwardDeclarations/UniquePtr.h>
#include <Common/Memory/AddressOf.h>

namespace ngine
{
	template<typename ContainedType>
	struct TRIVIAL_ABI UniqueRef
	{
		UniqueRef(UniquePtr<ContainedType>&&) = delete;
		UniqueRef& operator=(UniquePtr<ContainedType>&&) = delete;

		explicit UniqueRef(ContainedType* pElement) noexcept
			: m_pElement(pElement)
		{
			Assert(pElement != nullptr);
		}

		template<typename... Args>
		constexpr UniqueRef(Memory::ConstructInPlaceType, Args&&... args)
			: m_pElement(new ContainedType(Forward<Args>(args)...))
		{
		}

		UniqueRef(const UniqueRef&) = delete;
		UniqueRef& operator=(const UniqueRef&) = delete;

		UniqueRef(UniqueRef&& other) noexcept
			: m_pElement(other.ReleaseOwnership())
		{
		}

		UniqueRef& operator=(UniqueRef&& other) noexcept
		{
			m_pElement = other.ReleaseOwnership();
			return *this;
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		UniqueRef(UniqueRef<DerivedType>&& other) noexcept
			: m_pElement(static_cast<ContainedType*>(other.ReleaseOwnership()))
		{
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		UniqueRef& operator=(UniqueRef<DerivedType>&& other) noexcept
		{
			m_pElement = static_cast<ContainedType*>(other.ReleaseOwnership());
			return *this;
		}

		~UniqueRef() noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
			}
		}

		template<class... Args>
		[[nodiscard]] static UniqueRef Make(Args&&... args) noexcept
		{
			return UniqueRef(new ContainedType(Forward<Args>(args)...));
		}

		[[nodiscard]] static UniqueRef FromRaw(ContainedType* pElement) noexcept
		{
			return UniqueRef(pElement);
		}
		[[nodiscard]] static UniqueRef FromRaw(ContainedType& element) noexcept
		{
			return UniqueRef(Memory::GetAddressOf(element));
		}

		[[nodiscard]] PURE_STATICS NO_DEBUG ContainedType* Get() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG ContainedType& GetReference() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
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
		[[nodiscard]] PURE_STATICS NO_DEBUG operator ContainedType*() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG operator ContainedType&() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
		}

		template<class... Args>
		void CreateInPlace(Args&&... args) noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
			}

			m_pElement = new ContainedType(Forward<Args>(args)...);
		}

		[[nodiscard]] PURE_STATICS NO_DEBUG bool operator==(const ContainedType* pElement) const
		{
			return m_pElement == pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool operator!=(const ContainedType* pElement) const
		{
			return m_pElement != pElement;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool operator==(const ContainedType& element) const
		{
			return m_pElement == &element;
		}
		[[nodiscard]] PURE_STATICS NO_DEBUG bool operator!=(const ContainedType& element) const
		{
			return m_pElement != &element;
		}

		void DestroyElement() noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
				m_pElement = nullptr;
			}
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_pElement != nullptr;
		}

		[[nodiscard]] ContainedType* StealOwnership() noexcept
		{
			Expect(m_pElement != nullptr);
			ContainedType* pElement = m_pElement;
			m_pElement = nullptr;
			return pElement;
		}
	private:
		template<typename OtherType>
		friend struct UniqueRef;

		[[nodiscard]] ContainedType* ReleaseOwnership() noexcept
		{
			ContainedType* pElement = m_pElement;
			m_pElement = nullptr;
			return pElement;
		}
	protected:
		ContainedType* m_pElement;
	};
}

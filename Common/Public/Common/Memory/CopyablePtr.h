#pragma once

#include <Common/Assert/Assert.h>
#include "Forward.h"
#include "Move.h"
#include "UniquePtrView.h"

#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Memory/Invalid.h>

#include <Common/Memory/ForwardDeclarations/UniqueRef.h>

namespace ngine
{
	template<typename ContainedType>
	struct CopyablePtr
	{
		using View = UniquePtrView<ContainedType>;
		using ConstView = UniquePtrView<const ContainedType>;

		constexpr CopyablePtr() = default;
		constexpr CopyablePtr(decltype(nullptr)) noexcept
		{
		}
		constexpr CopyablePtr(InvalidType) noexcept
		{
		}
		CopyablePtr(UniqueRef<ContainedType>&& uniqueRef) = delete;
		CopyablePtr& operator=(UniqueRef<ContainedType>&&) = delete;
		explicit CopyablePtr(ContainedType* pElement) noexcept
			: m_pElement(pElement)
		{
			Assert(pElement != nullptr);
		}

		CopyablePtr(const CopyablePtr& other)
			: m_pElement(new ContainedType(*other.m_pElement))
		{
			Expect(other.m_pElement != nullptr);
		}
		CopyablePtr& operator=(const CopyablePtr& other)
		{
			DestroyElement();
			Expect(other.m_pElement != nullptr);
			m_pElement = new ContainedType(*other.m_pElement);
			return *this;
		}

		CopyablePtr(CopyablePtr&& other) noexcept
			: m_pElement(other.ReleaseOwnership())
		{
		}

		CopyablePtr& operator=(CopyablePtr&& other) noexcept
		{
			m_pElement = other.ReleaseOwnership();
			return *this;
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		CopyablePtr(CopyablePtr<DerivedType>&& other) noexcept
			: m_pElement(static_cast<ContainedType*>(other.ReleaseOwnership()))
		{
		}

		template<
			typename DerivedType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsBaseOf<ElementType, DerivedType>>>
		CopyablePtr& operator=(CopyablePtr<DerivedType>&& other) noexcept
		{
			m_pElement = static_cast<ContainedType*>(other.ReleaseOwnership());
			return *this;
		}

		~CopyablePtr() noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
			}
		}

		template<class... Args>
		[[nodiscard]] static CopyablePtr Make(Args&&... args) noexcept
		{
			return CopyablePtr(new ContainedType(Forward<Args>(args)...));
		}

		[[nodiscard]] PURE_STATICS ContainedType* Get() const noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS ContainedType& GetReference() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
		}
		[[nodiscard]] PURE_STATICS bool IsValid() const noexcept
		{
			return m_pElement != nullptr;
		}
		[[nodiscard]] PURE_STATICS bool IsInvalid() const noexcept
		{
			return m_pElement == nullptr;
		}
		[[nodiscard]] PURE_STATICS ContainedType* operator->() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS ContainedType& operator*() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
		}
		[[nodiscard]] operator ContainedType*() const noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS operator View() noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS operator ConstView() const noexcept LIFETIME_BOUND
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS constexpr operator bool() const noexcept
		{
			return m_pElement != nullptr;
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

		void DestroyElement() noexcept
		{
			if (m_pElement != nullptr)
			{
				delete m_pElement;
				m_pElement = nullptr;
			}
		}

		[[nodiscard]] ContainedType&& StealOwnership() noexcept
		{
			Expect(m_pElement != nullptr);
			ContainedType* pElement = m_pElement;
			m_pElement = nullptr;
			return Move(*pElement);
		}
	protected:
		template<typename OtherType>
		friend struct CopyablePtr;

		[[nodiscard]] ContainedType* ReleaseOwnership() noexcept
		{
			ContainedType* pElement = m_pElement;
			m_pElement = nullptr;
			return pElement;
		}
	protected:
		ContainedType* m_pElement = nullptr;
	};
}

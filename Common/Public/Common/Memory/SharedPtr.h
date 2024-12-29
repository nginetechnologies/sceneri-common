#pragma once

#include <Common/Assert/Assert.h>
#include <Common/Memory/Forward.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Threading/AtomicInteger.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename ContainedType, typename ReferenceCountType = Threading::Atomic<size>>
	struct TRIVIAL_ABI SharedPtr
	{
	protected:
		struct TrackedContainedType final : public ContainedType
		{
			inline static constexpr bool IsAtomic = TypeTraits::IsSame<ReferenceCountType, Threading::Atomic<size>>;

			template<class... Args>
			TrackedContainedType(Args&&... args)
				: ContainedType(Forward<Args>(args)...)
			{
			}

			void AcquireReference() noexcept
			{
				if constexpr (IsAtomic)
				{
					[[maybe_unused]] const auto previousValue = m_referenceCount.FetchAdd(1);
					Assert(previousValue > 0);
				}
				else
				{
					++m_referenceCount;
				}
			}

			size ReleaseReference() noexcept
			{
				if constexpr (IsAtomic)
				{
					const auto previousValue = m_referenceCount.FetchSubtract(1);
					Assert(previousValue > 0);
					if (previousValue == 1)
					{
						delete this;
					}
					return previousValue;
				}
				else
				{
					const size previousValue = m_referenceCount--;
					Assert(previousValue > 0);
					if (previousValue == 1)
					{
						delete this;
					}
					return previousValue;
				}
			}

			ReferenceCountType m_referenceCount = 1;
		};
	public:
		SharedPtr() = default;
		SharedPtr(const SharedPtr& other) noexcept
			: m_pElement(other.m_pElement)
		{
			if (m_pElement != nullptr)
			{
				m_pElement->AcquireReference();
			}
		}
		void Swap(SharedPtr& other) noexcept
		{
			TrackedContainedType* pTmp = m_pElement;
			m_pElement = other.m_pElement;
			other.m_pElement = pTmp;
		}
		SharedPtr& operator=(SharedPtr other) noexcept
		{
			Swap(other);
			return *this;
		}
		SharedPtr(SharedPtr&& other) noexcept
			: m_pElement(other.m_pElement)
		{
			other.m_pElement = nullptr;
		}
		~SharedPtr() noexcept
		{
			if (m_pElement != nullptr)
			{
				m_pElement->ReleaseReference();
			}
		}

		template<class... Args>
		[[nodiscard]] static SharedPtr Make(Args&&... args) noexcept
		{
			return SharedPtr(new TrackedContainedType(Forward<Args>(args)...));
		}

		[[nodiscard]] bool IsValid() const noexcept
		{
			return m_pElement != nullptr;
		}
		[[nodiscard]] ContainedType* operator->() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return m_pElement;
		}
		[[nodiscard]] ContainedType& operator*() const noexcept LIFETIME_BOUND
		{
			Expect(m_pElement != nullptr);
			return *m_pElement;
		}

		[[nodiscard]] operator bool() const noexcept
		{
			return m_pElement != nullptr;
		}

		//! Resets the reference to the managed object, and returns the reference count before the change
		[[nodiscard]] size FetchReset() noexcept
		{
			if (m_pElement != nullptr)
			{
				return m_pElement->ReleaseReference();
			}
			return 0;
		}
	protected:
		SharedPtr(TrackedContainedType* pTrackedType) noexcept
			: m_pElement(pTrackedType)
		{
		}

		TrackedContainedType* m_pElement = nullptr;
	};
}

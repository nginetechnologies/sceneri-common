#pragma once

#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Memory/New.h>
#include <Common/Assert/Assert.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/Memory/Containers/ArrayView.h>
#include "Common/Math/NumericLimits.h"
#include <Common/Memory/Allocators/Allocate.h>
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/WithoutConst.h>

#include "ForwardDeclarations/DynamicAllocator.h"

namespace ngine::Memory
{
	template<typename AllocatedType, typename SizeType_, typename IndexType_>
	struct TRIVIAL_ABI DynamicAllocator
	{
		using SizeType = SizeType_;
		using DataSizeType = size;
		using IndexType = IndexType_;
		using View = ArrayView<AllocatedType, SizeType, IndexType, AllocatedType>;
		using ConstView = ArrayView<const AllocatedType, SizeType, IndexType, const AllocatedType>;
		using RestrictedView = typename View::RestrictedView;
		using ConstRestrictedView = typename ConstView::ConstRestrictedView;
		using ElementType = AllocatedType;

		template<typename OtherType>
		using Rebind = DynamicAllocator<OtherType, SizeType, IndexType>;

		inline static constexpr bool IsWritable = !TypeTraits::IsConst<AllocatedType>;
		inline static constexpr bool IsGrowable = true;

		DynamicAllocator() = default;
		template<typename CapacitySizeType>
		inline DynamicAllocator(const ReserveType, const CapacitySizeType capacity) noexcept
			: m_pData(StaticAllocate(static_cast<size>(capacity)))
			, m_capacity(static_cast<SizeType>(capacity))
		{
			Assert((size)capacity <= (size)GetTheoreticalCapacity(), "Tried to reserve past a container's theoretical capacity");
		}
		DynamicAllocator(const DynamicAllocator& other)
			: m_pData(StaticAllocate(other.m_capacity))
			, m_capacity(other.m_capacity)
		{
			GetView().CopyFrom(other.GetView());
		}
		DynamicAllocator& operator=(const DynamicAllocator& other)
		{
			m_pData = StaticAllocate(other.m_capacity);
			m_capacity = other.m_capacity;
			GetView().CopyFrom(other.GetView());
			return *this;
		}
		inline DynamicAllocator(DynamicAllocator&& other) noexcept
			: m_pData(other.m_pData)
			, m_capacity(other.m_capacity)
		{
			other.m_pData = nullptr;
			other.m_capacity = 0;
		}
		inline DynamicAllocator& operator=(DynamicAllocator&& other) noexcept LIFETIME_BOUND
		{
			StaticDeallocate(m_pData);

			m_capacity = other.m_capacity;
			m_pData = other.m_pData;
			other.m_pData = nullptr;
			other.m_capacity = 0;
			return *this;
		}
		inline ~DynamicAllocator()
		{
			StaticDeallocate(m_pData);
		}

		[[nodiscard]] PURE_STATICS AllocatedType* GetData() noexcept LIFETIME_BOUND
		{
			return m_pData;
		}
		[[nodiscard]] PURE_STATICS const AllocatedType* GetData() const noexcept LIFETIME_BOUND
		{
			return m_pData;
		}
		[[nodiscard]] PURE_STATICS View GetView() noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS ConstView GetView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS RestrictedView GetRestrictedView() noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS ConstRestrictedView GetRestrictedView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS SizeType GetCapacity() const noexcept
		{
			return m_capacity;
		}
		[[nodiscard]] PURE_STATICS static constexpr SizeType GetTheoreticalCapacity() noexcept
		{
			return Math::NumericLimits<SizeType>::Max;
		}
		[[nodiscard]] PURE_STATICS bool HasAnyCapacity() const noexcept
		{
			return m_capacity != 0;
		}

		inline void Allocate(const SizeType newCapacity) noexcept
		{
			Assert(newCapacity != m_capacity);
			m_capacity = newCapacity;

			if (m_pData == nullptr)
			{
				m_pData = StaticAllocate(m_capacity);
			}
			else
			{
				m_pData = StaticReallocate(m_pData, m_capacity);
			}
		}

		inline void Free() noexcept
		{
			if (m_pData != nullptr)
			{
				StaticDeallocate(m_pData);
				m_pData = nullptr;
				m_capacity = 0;
			}
		}

		[[nodiscard]] PURE_STATICS static constexpr bool IsDynamicallyStored()
		{
			return true;
		}

		[[nodiscard]] RESTRICTED_RETURN inline static AllocatedType* StaticAllocate(const size requiredSize) noexcept
		{
			if constexpr (alignof(AllocatedType) > sizeof(void*))
			{
				return static_cast<AllocatedType*>(Memory::AllocateAligned(requiredSize * sizeof(AllocatedType), alignof(AllocatedType)));
			}
			else if constexpr (sizeof(AllocatedType) * GetTheoreticalCapacity() <= Memory::MaximumSmallAllocationSize)
			{
				return static_cast<AllocatedType*>(Memory::AllocateSmall(requiredSize * sizeof(AllocatedType)));
			}
			else
			{
				return static_cast<AllocatedType*>(Memory::Allocate(requiredSize * sizeof(AllocatedType)));
			}
		}

		[[nodiscard]] RESTRICTED_RETURN inline static AllocatedType* StaticReallocate(AllocatedType* pPointer, const size requiredSize) noexcept
		{
			if constexpr (alignof(AllocatedType) > sizeof(void*))
			{
				return static_cast<AllocatedType*>(Memory::ReallocateAligned(pPointer, requiredSize * sizeof(AllocatedType), alignof(AllocatedType))
				);
			}
			else
			{
				return static_cast<AllocatedType*>(Memory::Reallocate(pPointer, requiredSize * sizeof(AllocatedType)));
			}
		}

		inline static void StaticDeallocate(AllocatedType* pPointer) noexcept
		{
			if constexpr (alignof(AllocatedType) > sizeof(void*))
			{
				Memory::DeallocateAligned(pPointer, alignof(AllocatedType));
			}
			else
			{
				Memory::Deallocate(pPointer);
			}
		}
	protected:
		AllocatedType* m_pData = nullptr;
		SizeType m_capacity = 0;
	};
}

#pragma once

#include "ForwardDeclarations/DynamicInlineStorageAllocator.h"
#include <Common/Memory/Allocators/DynamicAllocator.h>
#include <Common/Memory/Allocators/FixedAllocator.h>

namespace ngine::Memory
{
	template<typename AllocatedType, size InlineCapacity, typename SizeType_, typename IndexType_>
	struct DynamicInlineStorageAllocator
	{
		using DynamicAllocator = DynamicAllocator<AllocatedType, SizeType_, IndexType_>;
		using FixedAllocator = FixedAllocator<AllocatedType, InlineCapacity, SizeType_, IndexType_>;

		using SizeType = typename DynamicAllocator::SizeType;
		using DataSizeType = typename DynamicAllocator::DataSizeType;
		using IndexType = typename DynamicAllocator::IndexType;
		using View = typename DynamicAllocator::View;
		using ConstView = typename DynamicAllocator::ConstView;
		using RestrictedView = typename View::RestrictedView;
		using ConstRestrictedView = typename ConstView::ConstRestrictedView;
		using ElementType = typename DynamicAllocator::ElementType;

		template<typename OtherType>
		using Rebind = DynamicInlineStorageAllocator<OtherType, InlineCapacity, SizeType, IndexType>;

		inline static constexpr bool IsWritable = DynamicAllocator::IsWritable;
		inline static constexpr bool IsGrowable = true;

		DynamicInlineStorageAllocator() = default;
		template<typename CapacitySizeType>
		inline DynamicInlineStorageAllocator(const ReserveType, const CapacitySizeType capacity) noexcept
			: m_capacity(static_cast<SizeType>(capacity))
		{
			if (capacity > InlineCapacity)
			{
				m_pData = DynamicAllocator::StaticAllocate(m_capacity);
			}
			Assert((size)capacity <= (size)GetTheoreticalCapacity(), "Tried to reserve past a container's theoretical capacity");
		}
		DynamicInlineStorageAllocator(const DynamicInlineStorageAllocator&) = delete;
		DynamicInlineStorageAllocator& operator=(const DynamicInlineStorageAllocator&) = delete;
		inline DynamicInlineStorageAllocator(DynamicInlineStorageAllocator&& other) noexcept
			: m_capacity(other.m_capacity)
		{
			if (m_capacity <= InlineCapacity)
			{
				GetView().CopyFrom(other.GetView());
			}
			else
			{
				m_pData = other.m_pData;
			}
			other.m_pData = nullptr;
			other.m_capacity = 0;
		}
		inline DynamicInlineStorageAllocator& operator=(DynamicInlineStorageAllocator&& other) noexcept LIFETIME_BOUND
		{
			if (m_capacity > InlineCapacity)
			{
				DynamicAllocator::StaticDeallocate(m_pData);
			}

			m_capacity = other.m_capacity;
			if (m_capacity <= InlineCapacity)
			{
				GetView().CopyFrom(other.GetView());
			}
			else
			{
				m_pData = other.m_pData;
			}
			other.m_pData = nullptr;
			other.m_capacity = 0;
			return *this;
		}
		inline ~DynamicInlineStorageAllocator()
		{
			if (m_capacity > InlineCapacity)
			{
				DynamicAllocator::StaticDeallocate(m_pData);
			}
		}

		void Free()
		{
			if (m_capacity > InlineCapacity)
			{
				DynamicAllocator::StaticDeallocate(m_pData);
			}
			m_capacity = 0u;
		}

		[[nodiscard]] PURE_STATICS AllocatedType* GetData() noexcept LIFETIME_BOUND
		{
			if (m_capacity == 0)
			{
				return nullptr;
			}
			else if (m_capacity <= InlineCapacity)
			{
				return m_fixedAllocator.GetData();
			}
			return m_pData;
		}
		[[nodiscard]] PURE_STATICS const AllocatedType* GetData() const noexcept LIFETIME_BOUND
		{
			if (m_capacity == 0)
			{
				return nullptr;
			}
			else if (m_capacity <= InlineCapacity)
			{
				return m_fixedAllocator.GetData();
			}
			return m_pData;
		}
		[[nodiscard]] PURE_STATICS constexpr View GetView() noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS constexpr ConstView GetView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS constexpr RestrictedView GetRestrictedView() noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS constexpr ConstRestrictedView GetRestrictedView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), m_capacity};
		}
		[[nodiscard]] PURE_STATICS constexpr SizeType GetCapacity() const noexcept
		{
			return m_capacity;
		}
		[[nodiscard]] PURE_STATICS static constexpr SizeType GetTheoreticalCapacity() noexcept
		{
			return Math::NumericLimits<SizeType>::Max;
		}
		[[nodiscard]] PURE_STATICS constexpr bool HasAnyCapacity() const noexcept
		{
			return m_capacity != 0;
		}

		inline void Allocate(const SizeType newCapacity) noexcept
		{
			Assert(newCapacity != m_capacity || newCapacity <= InlineCapacity);
			const SizeType previousCapacity = m_capacity;
			m_capacity = newCapacity;

			if (newCapacity > InlineCapacity)
			{
				if (previousCapacity <= InlineCapacity)
				{
					AllocatedType* pNewData = DynamicAllocator::StaticAllocate(m_capacity);
					Memory::CopyWithoutOverlap(pNewData, m_fixedAllocator.GetData(), previousCapacity * sizeof(AllocatedType));
					m_pData = pNewData;
				}
				else
				{
					m_pData = DynamicAllocator::StaticReallocate(m_pData, m_capacity);
				}
			}
		}

		[[nodiscard]] PURE_STATICS constexpr bool IsInlineStored() const
		{
			return m_capacity <= InlineCapacity;
		}

		[[nodiscard]] PURE_STATICS constexpr bool IsDynamicallyStored()
		{
			return m_capacity > InlineCapacity;
		}
	protected:
		union
		{
			AllocatedType* m_pData;
			FixedAllocator m_fixedAllocator;
		};
		SizeType m_capacity{0};
	};
}

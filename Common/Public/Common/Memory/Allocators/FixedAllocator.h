#pragma once

#include <Common/Platform/LifetimeBound.h>
#include <Common/Memory/New.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/Assert/Assert.h>
#include <Common/Memory/Containers/FixedArrayView.h>
#include <Common/Platform/Unused.h>
#include <Common/Platform/TrivialABI.h>

#include "ForwardDeclarations/FixedAllocator.h"

namespace ngine::Memory
{
	template<typename AllocatedType, size Capacity, typename SizeType_, typename IndexType_, size Alignment>
	struct alignas(Alignment) FixedAllocator
	{
		using SizeType = SizeType_;
		using DataSizeType = Memory::NumericSize<Capacity * sizeof(AllocatedType)>;
		using IndexType = IndexType_;
		using View = FixedArrayView<AllocatedType, Capacity, IndexType, SizeType_>;
		using ConstView = FixedArrayView<const AllocatedType, Capacity, IndexType, SizeType_>;
		using RestrictedView = typename View::RestrictedView;
		using ConstRestrictedView = typename View::ConstRestrictedView;
		using ElementType = AllocatedType;

		template<typename OtherType>
		using Rebind = FixedAllocator<OtherType, Capacity>;

		inline static constexpr bool IsGrowable = false;

		constexpr FixedAllocator() = default;
		template<typename CapacitySizeType>
		constexpr FixedAllocator(const ReserveType, const CapacitySizeType capacity) noexcept
		{
			UNUSED(capacity);
			Assert(capacity <= GetTheoreticalCapacity());
		}
		FixedAllocator(const Memory::ZeroedType)
		{
			GetView().ZeroInitialize();
		}
		FixedAllocator(const Memory::InitializeAllType)
		{
			GetView().DefaultConstruct();
		}
		FixedAllocator(const FixedAllocator&) = delete;
		FixedAllocator& operator=(const FixedAllocator&) = delete;
		FixedAllocator(FixedAllocator&& other) = default;
		FixedAllocator& operator=(FixedAllocator&& other) LIFETIME_BOUND = default;

		[[nodiscard]] PURE_STATICS AllocatedType* GetData() noexcept LIFETIME_BOUND
		{
			return reinterpret_cast<AllocatedType*>(&m_elementStorage);
		}
		[[nodiscard]] PURE_STATICS const AllocatedType* GetData() const noexcept LIFETIME_BOUND
		{
			return reinterpret_cast<const AllocatedType*>(&m_elementStorage);
		}
		[[nodiscard]] PURE_STATICS View GetView() noexcept LIFETIME_BOUND
		{
			using ArrayType = AllocatedType(&)[Capacity];
			return reinterpret_cast<ArrayType>(m_elementStorage);
		}
		[[nodiscard]] PURE_STATICS ConstView GetView() const noexcept LIFETIME_BOUND
		{
			using ArrayType = const AllocatedType(&)[Capacity];
			return reinterpret_cast<const ArrayType>(m_elementStorage);
		}
		[[nodiscard]] PURE_STATICS RestrictedView GetRestrictedView() noexcept LIFETIME_BOUND
		{
			using ArrayType = AllocatedType(&)[Capacity];
			return reinterpret_cast<ArrayType>(m_elementStorage);
		}
		[[nodiscard]] PURE_STATICS ConstRestrictedView GetRestrictedView() const noexcept LIFETIME_BOUND
		{
			using ArrayType = const AllocatedType(&)[Capacity];
			return reinterpret_cast<const ArrayType>(m_elementStorage);
		}
		[[nodiscard]] PURE_STATICS static constexpr SizeType GetCapacity() noexcept
		{
			return Capacity;
		}
		[[nodiscard]] PURE_STATICS static constexpr SizeType GetTheoreticalCapacity() noexcept
		{
			return Capacity;
		}
		[[nodiscard]] PURE_STATICS static constexpr bool HasAnyCapacity() noexcept
		{
			return true;
		}

		constexpr void Allocate(const SizeType newCapacity) noexcept
		{
			UNUSED(newCapacity);
			Assert(newCapacity <= Capacity, "Tried to resize fixed array beyond capacity!");
		}

		[[nodiscard]] PURE_STATICS static constexpr bool IsDynamicallyStored()
		{
			return false;
		}
	protected:
		struct
		{
			alignas(Alignment) ByteType m_elementStorage[sizeof(AllocatedType) * Capacity];
		} m_elementStorage;
	};

	template<typename AllocatedType, typename SizeType_, typename IndexType_, size Alignment>
	struct TRIVIAL_ABI alignas(Alignment) FixedAllocator<AllocatedType, 0, SizeType_, IndexType_, Alignment>
	{
		using SizeType = SizeType_;
		using DataSizeType = uint8;
		using IndexType = IndexType_;
		using View = FixedArrayView<AllocatedType, 0, IndexType, SizeType_>;
		using ConstView = FixedArrayView<const AllocatedType, 0, IndexType, SizeType_>;
		using RestrictedView = typename View::RestrictedView;
		using ConstRestrictedView = typename View::ConstRestrictedView;
		using ElementType = AllocatedType;

		template<typename OtherType>
		using Rebind = FixedAllocator<OtherType, 0>;

		inline static constexpr bool IsGrowable = false;

		constexpr FixedAllocator() = default;
		template<typename CapacitySizeType>
		constexpr FixedAllocator(const ReserveType, const CapacitySizeType capacity) noexcept
		{
			UNUSED(capacity);
			Assert(capacity == 0);
		}
		constexpr FixedAllocator(const Memory::ZeroedType)
		{
		}
		constexpr FixedAllocator(const Memory::InitializeAllType)
		{
		}
		FixedAllocator(const FixedAllocator&) = delete;
		FixedAllocator& operator=(const FixedAllocator&) = delete;
		FixedAllocator(FixedAllocator&& other) = default;
		FixedAllocator& operator=(FixedAllocator&& other) LIFETIME_BOUND = default;
		~FixedAllocator() = default;

		[[nodiscard]] PURE_STATICS constexpr AllocatedType* GetData() noexcept LIFETIME_BOUND
		{
			return nullptr;
		}
		[[nodiscard]] PURE_STATICS constexpr const AllocatedType* GetData() const noexcept LIFETIME_BOUND
		{
			return nullptr;
		}
		[[nodiscard]] PURE_STATICS constexpr View GetView() noexcept LIFETIME_BOUND
		{
			return {};
		}
		[[nodiscard]] PURE_STATICS constexpr ConstView GetView() const noexcept LIFETIME_BOUND
		{
			return {};
		}
		[[nodiscard]] PURE_STATICS constexpr SizeType GetCapacity() const noexcept
		{
			return 0;
		}
		[[nodiscard]] PURE_STATICS static constexpr SizeType GetTheoreticalCapacity() noexcept
		{
			return 0;
		}
		[[nodiscard]] PURE_STATICS constexpr bool HasAnyCapacity() const noexcept
		{
			return false;
		}

		constexpr void Allocate(const SizeType newCapacity) noexcept
		{
			UNUSED(newCapacity);
			Assert(newCapacity == 0, "Tried to resize fixed array beyond capacity!");
		}

		[[nodiscard]] PURE_STATICS static constexpr bool IsDynamicallyStored()
		{
			return false;
		}
	};
}

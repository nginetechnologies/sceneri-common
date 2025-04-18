#pragma once

#include "../VectorBase.h"
#include <Common/Compression/Compress.h>
#include <Common/Compression/Decompress.h>
#include <Common/Compression/Compressor.h>
#include <Common/Memory/Containers/BitView.h>
#include <Common/Memory/CountBits.h>

namespace ngine
{
	template<typename ContainedType, typename AllocatorType, uint8 Flags>
	inline constexpr uint32 TVector<ContainedType, AllocatorType, Flags>::CalculateCompressedDataSize() const
	{
		constexpr auto MaximumCount = AllocatorType::GetTheoreticalCapacity();
		constexpr auto SizeBitCount = Memory::GetBitWidth(MaximumCount);

		if constexpr (Compression::IsDynamicallyCompressed<ContainedType>())
		{
			uint32 dataSize = SizeBitCount + GetSize() * Compression::CalculateFixedDataSize<ContainedType>();
			for (const ContainedType& element : *this)
			{
				dataSize += Compression::CalculateDynamicDataSize<ContainedType>(element);
			}

			return dataSize;
		}
		else
		{
			return SizeBitCount + GetSize() * Compression::CalculateFixedDataSize<ContainedType>();
		}
	}

	template<typename ContainedType, typename AllocatorType, uint8 Flags>
	inline bool TVector<ContainedType, AllocatorType, Flags>::Compress(BitView& target) const
	{
		constexpr auto MaximumCount = AllocatorType::GetTheoreticalCapacity();
		constexpr auto SizeBitCount = Memory::GetBitWidth(MaximumCount);

		const SizeType elementCount = GetSize();
		bool wasPacked = target.PackAndSkip(ConstBitView::Make(elementCount, Math::Range<size>::Make(0, SizeBitCount)));

		for (const ContainedType& element : *this)
		{
			wasPacked &= Compression::Compress(element, target);
		}
		return wasPacked;
	}

	template<typename ContainedType, typename AllocatorType, uint8 Flags>
	inline bool TVector<ContainedType, AllocatorType, Flags>::Decompress(ConstBitView& source)
	{
		constexpr auto MaximumCount = AllocatorType::GetTheoreticalCapacity();
		constexpr auto SizeBitCount = Memory::GetBitWidth(MaximumCount);

		SizeType elementCount;
		bool success = source.UnpackAndSkip(BitView::Make(elementCount, Math::Range<size>::Make(0, SizeBitCount)));
		Clear();
		Reserve(elementCount);

		for (SizeType elementIndex = 0; elementIndex < elementCount; ++elementIndex)
		{
			ContainedType& target = EmplaceBack();
			success &= Compression::Decompress(target, source);
		}

		return success;
	}
}

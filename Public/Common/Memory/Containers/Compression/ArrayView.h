#pragma once

#include "../ArrayView.h"
#include <Common/Compression/Compress.h>
#include <Common/Compression/Compressor.h>
#include <Common/Memory/Containers/BitView.h>

namespace ngine
{
	template<typename ContainedType, typename SizeType, typename StoredType, typename IndexType, uint8 Flags>
	inline constexpr uint32 ArrayView<ContainedType, SizeType, StoredType, IndexType, Flags>::CalculateCompressedDataSize() const
	{
		constexpr uint32 indexDataSize = sizeof(IndexType) * 8;
		if constexpr (Compression::IsDynamicallyCompressed<ContainedType>())
		{
			uint32 dataSize = indexDataSize + GetSize() * Compression::CalculateFixedDataSize<ContainedType>();
			for (const ContainedType& element : *this)
			{
				dataSize += Compression::CalculateDynamicDataSize<ContainedType>(element);
			}

			return dataSize;
		}
		else
		{
			return indexDataSize + GetSize() * Compression::CalculateFixedDataSize<ContainedType>();
		}
	}

	template<typename ContainedType, typename SizeType, typename StoredType, typename IndexType, uint8 Flags>
	inline bool ArrayView<ContainedType, SizeType, StoredType, IndexType, Flags>::Compress(BitView& target) const
	{
		const SizeType elementCount = GetSize();
		bool wasPacked = target.PackAndSkip(ConstBitView::Make(elementCount));

		for (const ContainedType& element : *this)
		{
			wasPacked &= Compression::Compress(element, target);
		}
		return wasPacked;
	}
}

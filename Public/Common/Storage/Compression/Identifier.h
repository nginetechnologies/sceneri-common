#pragma once

#include "../Identifier.h"
#include <Common/Compression/Compressor.h>
#include <Common/Memory/Containers/BitView.h>

namespace ngine
{
	template<typename InternalType, uint8 IndexBitCount, InternalType MaximumCount>
	bool TIdentifier<InternalType, IndexBitCount, MaximumCount>::Compress(BitView& target) const
	{
		const IndexType index = GetIndex();
		return target.PackAndSkip(ConstBitView::Make(index, Math::Range<size>::Make(0, IndexBitCount)));
	}

	template<typename InternalType, uint8 IndexBitCount, InternalType MaximumCount>
	bool TIdentifier<InternalType, IndexBitCount, MaximumCount>::Decompress(ConstBitView& source)
	{
		IndexType index;
		const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, IndexBitCount)));
		*this = MakeFromIndex(index);
		return wasDecompressed;
	}
}

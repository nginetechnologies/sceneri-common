#pragma once

#include "../BitsetBase.h"
#include <Common/Compression/Compressor.h>
#include <Common/Memory/Containers/BitView.h>

namespace ngine::Memory
{
	template<typename AllocatorType, uint64 MaximumCount>
	bool BitsetBase<AllocatorType, MaximumCount>::Compress(BitView& target) const
	{
		return target.PackAndSkip(ConstBitView::Make(*this, Math::Range<size>::Make(0, MaximumCount)));
	}

	template<typename AllocatorType, uint64 MaximumCount>
	bool BitsetBase<AllocatorType, MaximumCount>::Decompress(ConstBitView& source)
	{
		return source.UnpackAndSkip(BitView::Make(*this, Math::Range<size>::Make(0, MaximumCount)));
	}
}

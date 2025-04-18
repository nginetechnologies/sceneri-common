#pragma once

#include <Common/Compression/ForwardDeclarations/Compressor.h>
#include <Common/Memory/Containers/BitView.h>
#include <Common/Math/NumericLimits.h>

namespace ngine::Compression
{
	template<>
	struct Compressor<bool>
	{
		using Type = bool;

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			return 1;
		}

		static bool Compress(const bool& source, BitView& target)
		{
			const uint8 value = source ? Math::NumericLimits<uint8>::Max : 0;
			return target.PackAndSkip(ConstBitView::Make(value, Math::Range<size>::Make(0, 1)));
		}

		static bool Decompress(bool& target, ConstBitView& source)
		{
			uint8 value;
			const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(value, Math::Range<size>::Make(0, 1)));
			target = value != 0;
			return wasDecompressed;
		}
	};
}

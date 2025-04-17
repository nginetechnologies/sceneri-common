#pragma once

#include <Common/Compression/ForwardDeclarations/Compressor.h>
#include <Common/Math/Range.h>
#include <Common/Math/Quantize.h>
#include <Common/TypeTraits/IsFloatingPoint.h>
#include <Common/Memory/Containers/Array.h>

namespace ngine::Compression
{
	template<typename Type_>
	struct Compressor<Type_, EnableIf<TypeTraits::IsFloatingPoint<Type_>>>
	{
		using Type = Type_;

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			return Math::NumericLimits<Type>::NumBits;
		}

		static bool Compress(const Type& source, BitView& target)
		{
			const Math::Range<Type> range = Math::Range<Type>::MakeStartToEnd(Type(-100000.0), Type(100000.0));
			Assert(range.Contains(source));
			const size bitCount = Math::NumericLimits<Type>::NumBits;
			const uint64 quantized = Math::Quantize(source, Math::QuantizationMode::Truncate, range, bitCount);
			return target.PackAndSkip(ConstBitView::Make(quantized, Math::Range<size>::Make(0, bitCount)));
		}

		static bool Decompress(Type& target, ConstBitView& source)
		{
			uint64 quantized;
			const size bitCount = Math::NumericLimits<Type>::NumBits;
			const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(quantized, Math::Range<size>::Make(0, bitCount)));
			const Math::Range<Type> range = Math::Range<Type>::MakeStartToEnd(Type(-100000.0), Type(100000.0));
			target = Math::Dequantize(quantized, range, bitCount);
			return wasDecompressed;
		}
	};
}

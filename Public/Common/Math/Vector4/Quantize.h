#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Quantize.h>
#include <Common/Memory/Containers/FixedArrayView.h>

namespace ngine::Math
{
	template<typename Type>
	[[nodiscard]] constexpr TVector4<uint32> Quantize(
		const TVector4<Type> source,
		const FixedArrayView<const QuantizationMode, 4> modes,
		const FixedArrayView<const Math::Range<Type>, 4> ranges,
		const FixedArrayView<const uint32, 4> bitCounts
	)
	{
		return {
			Quantize(source.x, modes[0], ranges[0], bitCounts[0]),
			Quantize(source.y, modes[1], ranges[1], bitCounts[1]),
			Quantize(source.z, modes[2], ranges[2], bitCounts[2]),
			Quantize(source.w, modes[3], ranges[3], bitCounts[3])
		};
	}

	template<typename Type>
	[[nodiscard]] constexpr TVector4<Type> Dequantize(
		const TVector4<uint32> source, const FixedArrayView<const Math::Range<Type>, 4> ranges, const FixedArrayView<const uint32, 4> bitCounts
	)
	{
		return {
			Dequantize(source.x, ranges[0], bitCounts[0]),
			Dequantize(source.y, ranges[1], bitCounts[1]),
			Dequantize(source.z, ranges[2], bitCounts[2]),
			Dequantize(source.w, ranges[3], bitCounts[3])
		};
	}
}

#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Quantize.h>
#include <Common/Memory/Containers/FixedArrayView.h>

namespace ngine::Math
{
	template<typename Type>
	[[nodiscard]] constexpr TVector3<uint32> Quantize(
		const TVector3<Type> source,
		const FixedArrayView<const QuantizationMode, 3> modes,
		const FixedArrayView<const Math::Range<Type>, 3> ranges,
		const FixedArrayView<const uint32, 3> bitCounts
	)
	{
		return {
			Quantize(source.x, modes[0], ranges[0], bitCounts[0]),
			Quantize(source.y, modes[1], ranges[1], bitCounts[1]),
			Quantize(source.z, modes[2], ranges[2], bitCounts[2])
		};
	}

	template<typename Type>
	[[nodiscard]] constexpr TVector3<Type> Dequantize(
		const TVector3<uint32> source, const FixedArrayView<const Math::Range<Type>, 3> ranges, const FixedArrayView<const uint32, 3> bitCounts
	)
	{
		return {
			Dequantize(source.x, ranges[0], bitCounts[0]),
			Dequantize(source.y, ranges[1], bitCounts[1]),
			Dequantize(source.z, ranges[2], bitCounts[2])
		};
	}
}

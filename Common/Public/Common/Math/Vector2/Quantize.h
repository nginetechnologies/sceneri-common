#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Quantize.h>
#include <Common/Memory/Containers/FixedArrayView.h>

namespace ngine::Math
{
	template<typename Type>
	[[nodiscard]] constexpr TVector2<uint32> Quantize(
		const TVector2<Type> source,
		const FixedArrayView<const QuantizationMode, 2> modes,
		const FixedArrayView<const Math::Range<Type>, 2> ranges,
		const FixedArrayView<const uint32, 2> bitCounts
	)
	{
		return {Quantize(source.x, modes[0], ranges[0], bitCounts[0]), Quantize(source.y, modes[1], ranges[1], bitCounts[1])};
	}

	template<typename Type>
	[[nodiscard]] constexpr TVector2<Type> Dequantize(
		const TVector2<uint32> source, const FixedArrayView<const Math::Range<Type>, 2> ranges, const FixedArrayView<const uint32, 2> bitCounts
	)
	{
		return {Dequantize(source.x, ranges[0], bitCounts[0]), Dequantize(source.y, ranges[1], bitCounts[1])};
	}
}

#pragma once

#include <Common/Math/Range.h>
#include <Common/Math/Ratio.h>
#include <Common/Math/MathAssert.h>
#include <Common/Memory/GetIntegerType.h>

namespace ngine::Math
{
	enum class QuantizationMode : uint8
	{
		//! Rounds towards zero
		Truncate,
		//! Rounds to the closest value, halfway cases away from zero
		Round,
		//! Always rounds to the upper value, ensuring we never receive a lower dequantized value
		AlwaysRoundUp
	};

	template<typename Type, typename QuantizedType = Memory::UnsignedIntegerType<sizeof(Type) * 8>>
	[[nodiscard]] constexpr QuantizedType
	Quantize(const Type source, const QuantizationMode mode, const Math::Range<Type> range, const uint32 bitCount)
	{
		MathExpect(bitCount <= Math::NumericLimits<QuantizedType>::NumBits);
		MathAssert(range.Contains(source));
		const Math::Ranged doubleRange = Math::Ranged::Make(range.GetMinimum(), range.GetSize());
		const Math::Ratiod ratio = doubleRange.GetClampedRatio(source);
		const QuantizedType maximumRepresentableValue = QuantizedType((1ull << bitCount) - 1ull);

		switch (mode)
		{
			case QuantizationMode::Truncate:
				return QuantizedType(double(maximumRepresentableValue) * ratio);
			case QuantizationMode::Round:
				return QuantizedType(double(maximumRepresentableValue) * ratio + Type(0.5));
			case QuantizationMode::AlwaysRoundUp:
				return QuantizedType(double(maximumRepresentableValue) * ratio + Type(1.0));
		}
		ExpectUnreachable();
	}

	template<typename Type, typename QuantizedType = Memory::UnsignedIntegerType<sizeof(Type) * 8>>
	[[nodiscard]] constexpr Type Dequantize(const QuantizedType source, const Math::Range<Type> range, const uint32 bitCount)
	{
		MathExpect(bitCount <= Math::NumericLimits<QuantizedType>::NumBits);
		const QuantizedType maximumRepresentableValue = QuantizedType((1ull << bitCount) - 1ull);
		const Type result = ((Type)source * range.GetRange()) / Type(maximumRepresentableValue) + range.GetMinimum();
		MathAssert(range.Contains(result));
		return result;
	}
}

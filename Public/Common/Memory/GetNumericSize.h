#pragma once

#include <Common/Math/NumericLimits.h>
#include <Common/TypeTraits/ConditionalType.h>

namespace ngine::Memory
{
	template<auto Value>
	using UnsignedNumericSize = TypeTraits::ConditionalType<
		(Value <= Math::NumericLimits<uint8>::Max),
		uint8,
		TypeTraits::ConditionalType<
			(Value <= Math::NumericLimits<uint16>::Max),
			uint16,
			TypeTraits::ConditionalType<
				(Value <= Math::NumericLimits<uint32>::Max),
				uint32,
				TypeTraits::ConditionalType<(Value <= Math::NumericLimits<uint64>::Max), uint64, void>>>>;

	template<auto Value>
	using SignedNumericSize = TypeTraits::ConditionalType<
		(Value <= Math::NumericLimits<uint8>::Max),
		int8,
		TypeTraits::ConditionalType<
			(Value <= Math::NumericLimits<uint16>::Max),
			int16,
			TypeTraits::ConditionalType<
				(Value <= Math::NumericLimits<uint32>::Max),
				int32,
				TypeTraits::ConditionalType<(Value <= Math::NumericLimits<uint64>::Max), int64, void>>>>;

	template<auto Size>
	using NumericSize = UnsignedNumericSize<Size>;
}

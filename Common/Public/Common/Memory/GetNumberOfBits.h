#pragma once

#include <Common/Memory/CountBits.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Platform/IsConstantEvaluated.h>

namespace ngine::Memory
{
	template<typename NumericType>
	[[nodiscard]] constexpr PURE_STATICS NumericType GetNumberOfBits(const NumericType x) noexcept
	{
		if (IsConstantEvaluated())
		{
			return x < 2 ? x : (NumericType)1 + GetNumberOfBits(NumericType(x >> 1));
		}
		else
		{
			return Math::NumericLimits<NumericType>::NumBits - Memory::GetNumberOfLeadingZeros(x);
		}
	}
}

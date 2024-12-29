#pragma once

#include <Common/Math/NumericLimits.h>
#include <Common/Math/Abs.h>
#include <Common/Math/MathAssert.h>
#include <Common/Memory/CountBits.h>
#include <Common/Math/MathAssert.h>
#include <Common/TypeTraits/IsSigned.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/TypeTraits/MakeUnsigned.h>

namespace ngine::Math
{
	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr bool IsPowerOfTwo(const Type value) noexcept
	{
		static_assert(TypeTraits::IsIntegral<Type>);
		MathExpect(value > 0);
		if constexpr (TypeTraits::IsUnsigned<Type>)
		{
			return (value & (value - 1)) == 0;
		}
		else
		{
			using UnsignedType = TypeTraits::Unsigned<Type>;
			const UnsignedType unsignedValue = (UnsignedType)Math::Abs(value);
			return (unsignedValue & (unsignedValue - 1)) == 0;
		}
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr T NearestPowerOfTwo(const T value) noexcept
	{
		constexpr T bitCount = sizeof(T) * 8;
		return value <= 1 ? T(1) : T(1) << (bitCount - Memory::GetNumberOfLeadingZeros(T(value - 1)));
	}
}

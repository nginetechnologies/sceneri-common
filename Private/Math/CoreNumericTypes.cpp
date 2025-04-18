#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/NumericLimits.h>

#include <cstdint>
#include <cstddef>
#include <limits>

#include "TypeTraits/IsSame.h"

namespace ngine::Math
{
	static_assert(NumericLimits<unsigned char>::Min == std::numeric_limits<unsigned char>::min());
	static_assert(NumericLimits<unsigned char>::Max == std::numeric_limits<unsigned char>::max());
	static_assert(NumericLimits<signed char>::Min == std::numeric_limits<signed char>::min());
	static_assert(NumericLimits<signed char>::Max == std::numeric_limits<signed char>::max());

	static_assert(NumericLimits<unsigned short>::Min == std::numeric_limits<unsigned short>::min());
	static_assert(NumericLimits<unsigned short>::Max == std::numeric_limits<unsigned short>::max());
	static_assert(NumericLimits<signed short>::Min == std::numeric_limits<signed short>::min());
	static_assert(NumericLimits<signed short>::Max == std::numeric_limits<signed short>::max());

	static_assert(NumericLimits<unsigned int>::Min == std::numeric_limits<unsigned int>::min());
	static_assert(NumericLimits<unsigned int>::Max == std::numeric_limits<unsigned int>::max());
	static_assert(NumericLimits<signed int>::Min == std::numeric_limits<signed int>::min());
	static_assert(NumericLimits<signed int>::Max == std::numeric_limits<signed int>::max());

	static_assert(NumericLimits<unsigned long>::Min == std::numeric_limits<unsigned long>::min());
	static_assert(NumericLimits<unsigned long>::Max == std::numeric_limits<unsigned long>::max());
	static_assert(NumericLimits<signed long>::Min == std::numeric_limits<signed long>::min());
	static_assert(NumericLimits<signed long>::Max == std::numeric_limits<signed long>::max());

	static_assert(NumericLimits<unsigned long long>::Min == std::numeric_limits<unsigned long long>::min());
	static_assert(NumericLimits<unsigned long long>::Max == std::numeric_limits<unsigned long long>::max());
	static_assert(NumericLimits<signed long long>::Min == std::numeric_limits<signed long long>::min());
	static_assert(NumericLimits<signed long long>::Max == std::numeric_limits<signed long long>::max());

	static_assert(NumericLimits<float>::Min == std::numeric_limits<float>::lowest());
	static_assert(NumericLimits<float>::MinPositive == std::numeric_limits<float>::min());
	static_assert(NumericLimits<float>::Max == std::numeric_limits<float>::max());

	static_assert(NumericLimits<double>::Min == std::numeric_limits<double>::lowest());
	static_assert(NumericLimits<double>::MinPositive == std::numeric_limits<double>::min());
	static_assert(NumericLimits<double>::Max == std::numeric_limits<double>::max());
}

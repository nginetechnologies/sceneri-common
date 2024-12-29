#pragma once

#include "Floor.h"
#include "Mod.h"
#include "Min.h"
#include "SignNonZero.h"
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsFloatingPoint.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr EnableIf<!TypeTraits::IsFloatingPoint<T>, T>
	Wrap(T value, const T min, const T max) noexcept
	{
		T range_size = max - min + 1;
		value += Math::Select(value < min, range_size * ((min - value) / range_size + 1), 0);
		return min + Math::Mod((value - min), range_size);
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr EnableIf<TypeTraits::IsFloatingPoint<T>, T>
	Wrap(const T value, const T min, const T max) noexcept
	{
		const T range = (max - min);
		const T valueInRange = value - min;
		const T valueRatio = valueInRange / range;
		const T adjustedRatio = Math::SignNonZero(value) * T((valueRatio > T(1)) | (valueRatio < T(0)));
		return value - adjustedRatio * range;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr T* Wrap(T* ptrValue, T* ptrMin, T* ptrMax) noexcept
	{
		intptr value = reinterpret_cast<intptr>(ptrValue);
		intptr min = reinterpret_cast<intptr>(ptrMin);
		intptr max = reinterpret_cast<intptr>(ptrMax);

		intptr range_size = max - min + sizeof(T);
		value += Math::Select(value < min, range_size * ((min - value) / range_size + sizeof(T)), 0);
		return reinterpret_cast<T*>(min + Math::Mod((value - min), range_size));
	}
}

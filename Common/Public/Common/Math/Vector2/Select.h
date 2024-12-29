#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Vectorization/Select.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T>
	Select(const bool condition, const TVector2<T> trueValue, const TVector2<T> falseValue) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Select(condition, trueValue.GetVectorized(), falseValue.GetVectorized()));
		}
		else
		{
			return {Select(condition, trueValue.x, falseValue.x), Select(condition, trueValue.y, falseValue.y)};
		}
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T>
	Select(const TVector2<T> condition, const TVector2<T> trueValue, const TVector2<T> falseValue) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Select(condition.GetVectorized(), trueValue.GetVectorized(), falseValue.GetVectorized()));
		}
		else
		{
			return {Select(condition.x, trueValue.x, falseValue.x), Select(condition.y, trueValue.y, falseValue.y)};
		}
	}
}

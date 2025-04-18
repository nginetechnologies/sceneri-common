#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Select.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T>
	Select(const bool condition, const TVector3<T> trueValue, const TVector3<T> falseValue) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Select(condition, trueValue.GetVectorized(), falseValue.GetVectorized()));
		}
		else
		{
			return {
				Select(condition, trueValue.x, falseValue.x),
				Select(condition, trueValue.y, falseValue.y),
				Select(condition, trueValue.z, falseValue.z)
			};
		}
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T>
	Select(const TVector3<T> condition, const TVector3<T> trueValue, const TVector3<T> falseValue) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Select(condition.GetVectorized(), trueValue.GetVectorized(), falseValue.GetVectorized()));
		}
		else
		{
			return {
				Select((bool)condition.x, trueValue.x, falseValue.x),
				Select((bool)condition.y, trueValue.y, falseValue.y),
				Select((bool)condition.z, trueValue.z, falseValue.z)
			};
		}
	}
}

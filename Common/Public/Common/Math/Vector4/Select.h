#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Vectorization/Select.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T>
	Select(const bool condition, const TVector4<T> trueValue, const TVector4<T> falseValue) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::Select(condition, trueValue.GetVectorized(), falseValue.GetVectorized()));
		}
		else
		{
			return {
				Select(condition, trueValue.x, falseValue.x),
				Select(condition, trueValue.y, falseValue.y),
				Select(condition, trueValue.z, falseValue.z),
				Select(condition, trueValue.w, falseValue.w)
			};
		}
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T>
	Select(const TVector4<T> condition, const TVector4<T> trueValue, const TVector4<T> falseValue) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::Select(condition.GetVectorized(), trueValue.GetVectorized(), falseValue.GetVectorized()));
		}
		else
		{
			return {
				Select(condition.x, trueValue.x, falseValue.x),
				Select(condition.y, trueValue.y, falseValue.y),
				Select(condition.z, trueValue.z, falseValue.z),
				Select(condition.w, trueValue.w, falseValue.w)
			};
		}
	}
}

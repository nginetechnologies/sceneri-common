#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Round.h>
#include <Common/Math/Vectorization/Round.h>
#include <Common/TypeTraits/IsFloatingPoint.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector4<T> Round(const TVector4<T> value) noexcept
	{
		if constexpr (TypeTraits::IsFloatingPoint<T>)
		{
			if constexpr (TVector4<T>::IsVectorized)
			{
				return TVector4<T>(Math::Round(value.GetVectorized()));
			}
			else
			{
				return {Math::Round(value.x), Math::Round(value.y), Math::Round(value.z), Math::Round(value.w)};
			}
		}
		else
		{
			return value;
		}
	}
}

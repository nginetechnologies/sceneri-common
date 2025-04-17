#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Round.h>
#include <Common/Math/Vectorization/Round.h>
#include <Common/TypeTraits/IsFloatingPoint.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector3<T> Round(const TVector3<T> value) noexcept
	{
		if constexpr (TypeTraits::IsFloatingPoint<T>)
		{
			if constexpr (TVector3<T>::IsVectorized)
			{
				return TVector3<T>(Math::Round(value.GetVectorized()));
			}
			else
			{
				return {Math::Round(value.x), Math::Round(value.y), Math::Round(value.z)};
			}
		}
		else
		{
			return value;
		}
	}
}

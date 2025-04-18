#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Power.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Power(const TVector3<T> base, const TVector3<T> exponent) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Power(base.GetVectorized(), exponent.GetVectorized()));
		}
		else
		{
			return {Power(base.x, exponent.x), Power(base.y, exponent.y), Power(base.z, exponent.z)};
		}
	}
}

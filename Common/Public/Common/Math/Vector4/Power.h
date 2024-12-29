#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Vectorization/Power.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Power(const TVector4<T> base, const TVector4<T> exponent) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::Power(base.GetVectorized(), exponent.GetVectorized()));
		}
		else
		{
			return {Power(base.x, exponent.x), Power(base.y, exponent.y), Power(base.z, exponent.z), Power(base.w, exponent.w)};
		}
	}
}

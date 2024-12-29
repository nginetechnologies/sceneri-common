#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/SinCos.h>
#include <Common/Math/Vectorization/SinCos.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> SinCos(const TVector3<T> value, TVector3<T>& cosOut) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized && USE_SVML)
		{
			return TVector3(Math::SinCos(value.GetVectorized(), cosOut.GetVectorized()));
		}
		else
		{
			return {SinCos(value.x, cosOut.x), SinCos(value.y, cosOut.y), SinCos(value.z, cosOut.z)};
		}
	}
}

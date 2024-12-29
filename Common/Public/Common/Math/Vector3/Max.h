#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Max.h>
#include <Common/Math/Vectorization/Max.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Max(const TVector3<T> a, const TVector3<T> b) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3<T>(Math::Max(a.GetVectorized(), b.GetVectorized()));
		}
		else
		{
			return {Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z)};
		}
	}
}

#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Min.h>
#include <Common/Math/Vectorization/Min.h>
#include <Common/Math/Vector3/Select.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Min(const TVector3<T> a, const TVector3<T> b) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3<T>(Math::Min(a.GetVectorized(), b.GetVectorized()));
		}
		else
		{
			return {Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z)};
		}
	}
}

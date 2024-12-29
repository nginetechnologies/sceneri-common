#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Floor.h>
#include <Common/Math/Vectorization/Floor.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Floor(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Floor(value.GetVectorized()));
		}
		else
		{
			return {Floor(value.x), Floor(value.y), Floor(value.z)};
		}
	}
}

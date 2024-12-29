#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Sin.h>
#include <Common/Math/Sin.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Sin(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Sin(value.GetVectorized()));
		}
		else
		{
			return {Sin(value.x), Sin(value.y), Sin(value.z)};
		}
	}
}

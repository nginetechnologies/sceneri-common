#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Sqrt.h>
#include <Common/Math/Sqrt.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Sqrt(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Sqrt(value.GetVectorized()));
		}
		else
		{
			return {Sqrt(value.x), Sqrt(value.y), Sqrt(value.z)};
		}
	}
}

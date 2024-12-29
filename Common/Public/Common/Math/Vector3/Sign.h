#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Sign.h>
#include <Common/Math/Vectorization/Sign.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Sign(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Sign(value.GetVectorized()));
		}
		else
		{
			return {Sign(value.x), Sign(value.y), Sign(value.z)};
		}
	}
}

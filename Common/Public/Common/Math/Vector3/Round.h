#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Round.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector3<T> Round(const TVector3<T> value) noexcept
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
}

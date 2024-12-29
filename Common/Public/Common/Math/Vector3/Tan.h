#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Tan.h>
#include <Common/Math/Tan.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Tan(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Tan(value.GetVectorized()));
		}
		else
		{
			return {Tan(value.x), Tan(value.y), Tan(value.z)};
		}
	}
}

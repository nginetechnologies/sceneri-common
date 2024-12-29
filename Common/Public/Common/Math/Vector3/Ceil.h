#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Vectorization/Ceil.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Ceil(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Ceil(value.GetVectorized()));
		}
		else
		{
			return {Ceil(value.x), Ceil(value.y), Ceil(value.z)};
		}
	}
}

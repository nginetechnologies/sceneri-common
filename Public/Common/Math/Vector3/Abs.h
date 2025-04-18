#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Vectorization/Abs.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Abs(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Abs(value.GetVectorized()));
		}
		else
		{
			return {Abs(value.x), Abs(value.y), Abs(value.z)};
		}
	}
}

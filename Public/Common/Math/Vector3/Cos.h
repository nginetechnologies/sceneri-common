#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Cos.h>
#include <Common/Math/Cos.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> Cos(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::Cos(value.GetVectorized()));
		}
		else
		{
			return {Cos(value.x), Cos(value.y), Cos(value.z)};
		}
	}
}

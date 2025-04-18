#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Vectorization/Sqrt.h>
#include <Common/Math/Sqrt.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Sqrt(const TVector4<T> value) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::Sqrt(value.GetVectorized()));
		}
		else
		{
			return {Sqrt(value.x), Sqrt(value.y), Sqrt(value.z), Sqrt(value.w)};
		}
	}
}

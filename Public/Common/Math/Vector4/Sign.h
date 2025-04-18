#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Sign.h>
#include <Common/Math/Vectorization/Sign.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Sign(const TVector4<T> value) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::Sign(value.GetVectorized()));
		}
		else
		{
			return {Sign(value.x), Sign(value.y), Sign(value.z), Sign(value.w)};
		}
	}
}

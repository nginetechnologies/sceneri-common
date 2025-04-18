#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Sign.h>
#include <Common/Math/Vectorization/Sign.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> Sign(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Sign(value.GetVectorized()));
		}
		else
		{
			return {Sign(value.x), Sign(value.y)};
		}
	}
}

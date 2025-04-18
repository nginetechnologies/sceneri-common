#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Vectorization/Sqrt.h>
#include <Common/Math/Sqrt.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> Sqrt(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Sqrt(value.GetVectorized()));
		}
		else
		{
			return {Sqrt(value.x), Sqrt(value.y)};
		}
	}
}

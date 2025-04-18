#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Max.h>
#include <Common/Math/Vectorization/Max.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> Max(const TVector2<T> a, const TVector2<T> b) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Max(a.GetVectorized(), b.GetVectorized()));
		}
		else
		{
			return {Max(a.x, b.x), Max(a.y, b.y)};
		}
	}
}

#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Min.h>
#include <Common/Math/Vectorization/Min.h>
#include <Common/Math/Vector2/Select.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> Min(const TVector2<T> a, const TVector2<T> b) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Min(a.GetVectorized(), b.GetVectorized()));
		}
		else
		{
			return {Min(a.x, b.x), Min(a.y, b.y)};
		}
	}
}

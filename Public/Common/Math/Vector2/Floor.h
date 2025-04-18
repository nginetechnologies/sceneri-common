#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Floor.h>
#include <Common/Math/Vectorization/Floor.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector2<T> Floor(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return Math::Floor(value.GetVectorized());
		}
		else
		{
			return {Math::Floor(value.x), Math::Floor(value.y)};
		}
	}
}

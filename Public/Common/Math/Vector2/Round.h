#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Round.h>
#include <Common/Math/Vectorization/Round.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector2<T> Round(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return Math::Round(value.GetVectorized());
		}
		else
		{
			return {Math::Round(value.x), Math::Round(value.y)};
		}
	}
}

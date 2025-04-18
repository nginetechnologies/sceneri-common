#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Vectorization/Abs.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> Abs(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::Abs(value.GetVectorized()));
		}
		else
		{
			return {Abs(value.x), Abs(value.y)};
		}
	}
}

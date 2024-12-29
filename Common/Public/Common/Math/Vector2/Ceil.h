#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Vectorization/Ceil.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE TVector2<T> Ceil(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return Math::Ceil(value.GetVectorized());
		}
		else
		{
			return {Math::Ceil(value.x), Math::Ceil(value.y)};
		}
	}
}

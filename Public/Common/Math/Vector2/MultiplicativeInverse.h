#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/MultiplicativeInverse.h>
#include <Common/Math/Vectorization/MultiplicativeInverse.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> MultiplicativeInverse(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::MultiplicativeInverse(value.GetVectorized()));
		}
		else
		{
			return {Math::MultiplicativeInverse(value.x), Math::MultiplicativeInverse(value.y)};
		}
	}
}

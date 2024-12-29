#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/SignNonZero.h>
#include <Common/Math/Vectorization/SignNonZero.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector2<T> SignNonZero(const TVector2<T> value) noexcept
	{
		if constexpr (TVector2<T>::IsVectorized)
		{
			return TVector2(Math::SignNonZero(value.GetVectorized()));
		}
		else
		{
			return {SignNonZero(value.x), SignNonZero(value.y)};
		}
	}
}

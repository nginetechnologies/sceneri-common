#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/SignNonZero.h>
#include <Common/Math/Vectorization/SignNonZero.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> SignNonZero(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::SignNonZero(value.GetVectorized()));
		}
		else
		{
			return {SignNonZero(value.x), SignNonZero(value.y), SignNonZero(value.z)};
		}
	}
}

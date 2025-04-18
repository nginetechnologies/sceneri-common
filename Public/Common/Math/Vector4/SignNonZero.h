#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/SignNonZero.h>
#include <Common/Math/Vectorization/SignNonZero.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> SignNonZero(const TVector4<T> value) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::SignNonZero(value.GetVectorized()));
		}
		else
		{
			return {SignNonZero(value.x), SignNonZero(value.y), SignNonZero(value.z), SignNonZero(value.w)};
		}
	}
}

#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Vectorization/Abs.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Abs(const TVector4<T> value) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return Math::Abs(value.m_vectorized);
		}
		else
		{
			return {Abs(value.x), Abs(value.y), Abs(value.z), Abs(value.w)};
		}
	}
}

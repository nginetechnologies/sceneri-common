#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Max.h>
#include <Common/Math/Vectorization/Max.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Max(const TVector4<T> a, const TVector4<T> b) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4<T>(Math::Max(a.m_vectorized, b.m_vectorized));
		}
		else
		{
			return {Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z), Max(a.w, b.w)};
		}
	}
}

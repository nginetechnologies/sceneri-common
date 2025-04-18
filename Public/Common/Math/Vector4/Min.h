#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Min.h>
#include <Common/Math/Vectorization/Min.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> Min(const TVector4<T> a, const TVector4<T> b) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4<T>(Math::Min(a.m_vectorized, b.m_vectorized));
		}
		else
		{
			return {Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z), Min(a.w, b.w)};
		}
	}
}

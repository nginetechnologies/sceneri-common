#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/MultiplicativeInverse.h>
#include <Common/Math/Vectorization/MultiplicativeInverse.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector4<T> MultiplicativeInverse(const TVector4<T> value) noexcept
	{
		if constexpr (TVector4<T>::IsVectorized)
		{
			return TVector4(Math::MultiplicativeInverse(value.GetVectorized()));
		}
		else
		{
			return {
				Math::MultiplicativeInverse(value.x),
				Math::MultiplicativeInverse(value.y),
				Math::MultiplicativeInverse(value.z),
				Math::MultiplicativeInverse(value.w)
			};
		}
	}
}

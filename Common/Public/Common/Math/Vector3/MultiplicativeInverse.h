#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/MultiplicativeInverse.h>
#include <Common/Math/Vectorization/MultiplicativeInverse.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE constexpr TVector3<T> MultiplicativeInverse(const TVector3<T> value) noexcept
	{
		if constexpr (TVector3<T>::IsVectorized)
		{
			return TVector3(Math::MultiplicativeInverse(value.GetVectorized()));
		}
		else
		{
			return {Math::MultiplicativeInverse(value.x), Math::MultiplicativeInverse(value.y), Math::MultiplicativeInverse(value.z)};
		}
	}
}

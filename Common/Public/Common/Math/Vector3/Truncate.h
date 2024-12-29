#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vectorization/Truncate.h>

namespace ngine::Math
{
	template<>
	[[nodiscard]] FORCE_INLINE TVector3<int32> Truncate<TVector3<int32>>(const TVector3<float> value) noexcept
	{
		return TVector3<int32>(Math::Truncate<TVector3<int32>::VectorizedType>(value.GetVectorized()));
	}
}

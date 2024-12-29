#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Vectorization/Truncate.h>

namespace ngine::Math
{
	template<>
	[[nodiscard]] FORCE_INLINE TVector4<int32> Truncate<TVector4<int32>>(const TVector4<float> value) noexcept
	{
		return TVector4<int32>(Math::Truncate<TVector4<int32>::VectorizedType>(value.GetVectorized()));
	}
}

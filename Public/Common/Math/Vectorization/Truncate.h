#pragma once

#include "Packed.h"

#include <Common/Math/Truncate.h>

namespace ngine::Math
{
	template<>
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<int32, 4>
	Truncate<Vectorization::Packed<int32, 4>>(const Vectorization::Packed<float, 4> value) noexcept
	{
#if USE_SSE2
		return _mm_cvttps_epi32(value);
#else
		return {Truncate<int32>(value[0]), Truncate<int32>(value[1]), Truncate<int32>(value[2]), Truncate<int32>(value[3])};
#endif
	}

	template<>
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<int32, 2>
	Truncate<Vectorization::Packed<int32, 2>>(const Vectorization::Packed<double, 2> value) noexcept
	{
		return {Truncate<int32>(value[0]), Truncate<int32>(value[1])};
	}

	template<>
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<int32, 4>
	Truncate<Vectorization::Packed<int32, 4>>(const Vectorization::Packed<double, 4> value) noexcept
	{
		return {Truncate<int32>(value[0]), Truncate<int32>(value[1]), Truncate<int32>(value[2]), Truncate<int32>(value[2])};
	}
}

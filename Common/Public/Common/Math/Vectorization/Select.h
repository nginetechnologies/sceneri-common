#pragma once

#include "Packed.h"

#include <Common/Math/Select.h>

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4>
	Select(const bool condition, const Vectorization::Packed<float, 4> trueValue, const Vectorization::Packed<float, 4> falseValue) noexcept
	{
#if USE_SSE4_1
		return _mm_blendv_ps(falseValue, trueValue, Vectorization::Packed<float, 4>(condition * 1.f));
#else
		return {
			Select(condition, trueValue[0], falseValue[0]),
			Select(condition, trueValue[1], falseValue[1]),
			Select(condition, trueValue[2], falseValue[2]),
			Select(condition, trueValue[3], falseValue[3])
		};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 4> Select(
		const Vectorization::Packed<float, 4> condition,
		const Vectorization::Packed<float, 4> trueValue,
		const Vectorization::Packed<float, 4> falseValue
	) noexcept
	{
#if USE_SSE4_1
		return _mm_blendv_ps(falseValue, trueValue, condition);
#else
		return {
			Select(condition[0], trueValue[0], falseValue[0]),
			Select(condition[1], trueValue[1], falseValue[1]),
			Select(condition[2], trueValue[2], falseValue[2]),
			Select(condition[3], trueValue[3], falseValue[3])
		};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2>
	Select(const bool condition, const Vectorization::Packed<double, 2> trueValue, const Vectorization::Packed<double, 2> falseValue) noexcept
	{
#if USE_SSE4_1
		return _mm_blendv_pd(falseValue, trueValue, Vectorization::Packed<double, 2>(condition * 1.0));
#else
		return {Select(condition, trueValue[0], falseValue[0]), Select(condition, trueValue[1], falseValue[1])};
#endif
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 2> Select(
		const Vectorization::Packed<double, 2> condition,
		const Vectorization::Packed<double, 2> trueValue,
		const Vectorization::Packed<double, 2> falseValue
	) noexcept
	{
#if USE_SSE4_1
		return _mm_blendv_pd(falseValue, trueValue, condition);
#else
		return {Select(condition[0], trueValue[0], falseValue[0]), Select(condition[1], trueValue[1], falseValue[1])};
#endif
	}

#if USE_AVX
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8>
	Select(const bool condition, const Vectorization::Packed<float, 8> trueValue, const Vectorization::Packed<float, 8> falseValue) noexcept
	{
		return _mm256_blendv_ps(falseValue, trueValue, Vectorization::Packed<float, 8>(condition * 1.f));
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 8> Select(
		const Vectorization::Packed<float, 8> condition,
		const Vectorization::Packed<float, 8> trueValue,
		const Vectorization::Packed<float, 8> falseValue
	) noexcept
	{
		return _mm256_blendv_ps(falseValue, trueValue, condition);
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4>
	Select(const bool condition, const Vectorization::Packed<double, 4> trueValue, const Vectorization::Packed<double, 4> falseValue) noexcept
	{
		return _mm256_blendv_pd(falseValue, trueValue, Vectorization::Packed<double, 4>(condition * 1.0));
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 4> Select(
		const Vectorization::Packed<double, 4> condition,
		const Vectorization::Packed<double, 4> trueValue,
		const Vectorization::Packed<double, 4> falseValue
	) noexcept
	{
		return _mm256_blendv_pd(falseValue, trueValue, condition);
	}
#endif

#if USE_AVX512
	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16>
	Select(const bool condition, const Vectorization::Packed<float, 16> trueValue, const Vectorization::Packed<float, 16> falseValue) noexcept
	{
		Vectorization::Packed<float, 16> packedCondition(condition * 1.f);
		return _mm512_or_ps(_mm512_and_ps(packedCondition, trueValue), _mm512_andnot_ps(packedCondition, falseValue));
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<float, 16> Select(
		const Vectorization::Packed<float, 16> condition,
		const Vectorization::Packed<float, 16> trueValue,
		const Vectorization::Packed<float, 16> falseValue
	) noexcept
	{
		return _mm512_or_ps(_mm512_and_ps(condition, trueValue), _mm512_andnot_ps(condition, falseValue));
	}

	[[nodiscard]] FORCE_INLINE Vectorization::Packed<double, 8> Select(
		const Vectorization::Packed<double, 8> condition,
		const Vectorization::Packed<double, 8> trueValue,
		const Vectorization::Packed<double, 8> falseValue
	) noexcept
	{
		return _mm512_or_pd(_mm512_and_ps(condition, trueValue), _mm512_andnot_ps(condition, falseValue));
	}
#endif
}

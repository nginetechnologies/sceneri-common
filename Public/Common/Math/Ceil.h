#pragma once

#include <Common/Platform/CompilerWarnings.h>
#include <Common/Math/Select.h>
#include <Common/Platform/IsConstantEvaluated.h>

#if USE_SSE4_1
#include <smmintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr float Ceil(const float value) noexcept
	{
		if (IsConstantEvaluated())
		{
			PUSH_CLANG_WARNINGS
			DISABLE_CLANG_WARNING("-Wfloat-equal");

			return static_cast<float>(Math::Select(
				static_cast<float>(static_cast<int32>(value)) == value,
				static_cast<int32>(value),
				static_cast<int32>(value) + (value > 0)
			));

			POP_CLANG_WARNINGS
		}
		else
		{
#if USE_SSE4_1
			__m128 result{0};
			return _mm_cvtss_f32(_mm_ceil_ss(result, _mm_set_ss(value)));
#else
			return ::ceilf(value);
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr double Ceil(const double value) noexcept
	{
		if (IsConstantEvaluated())
		{
			PUSH_CLANG_WARNINGS
			DISABLE_CLANG_WARNING("-Wfloat-equal");

			return static_cast<double>(Math::Select(
				static_cast<double>(static_cast<int32>(value)) == value,
				static_cast<int32>(value),
				static_cast<int32>(value) + (value > 0)
			));

			POP_CLANG_WARNINGS
		}
		else
		{
#if USE_SSE4_1
			__m128d result{0};
			return _mm_cvtsd_f64(_mm_ceil_sd(result, _mm_set_sd(value)));
#else
			return ::ceil(value);
#endif
		}
	}
}

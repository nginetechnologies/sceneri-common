#pragma once

#include <Common/Math/Select.h>
#include <Common/Math/Constants.h>
#include <Common/Math/MathAssert.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Platform/IsConstantEvaluated.h>
#include <Common/Platform/CompilerWarnings.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

PUSH_MSVC_WARNINGS
DISABLE_MSVC_WARNINGS(4714) // function 'unsigned int __vectorcall ngine::Math::Log2(unsigned int)' marked as __forceinline not inlined

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Power(const double base, const double exponent) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_pow_pd(_mm_set_sd(base), _mm_set_sd(exponent)));
#else
		return ::pow(base, exponent);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Power(const float base, const float exponent) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(base), _mm_set_ss(exponent)));
#else
		return ::powf(base, exponent);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr double Power(const double base, const int exponent) noexcept
	{
		if (IsConstantEvaluated())
		{
			return Math::Select((bool)exponent, base * Power(base, exponent - 1), 1);
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return __builtin_powi(base, exponent);
#else
			return Power(base, static_cast<double>(exponent));
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr float Power(const float base, const int exponent) noexcept
	{
		if (IsConstantEvaluated())
		{
			return Math::Select((bool)exponent, base * Power(base, exponent - 1), 1.0f);
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return __builtin_powif(base, exponent);
#else
			return Power(base, static_cast<float>(exponent));
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Power2(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_exp2_pd(_mm_set_sd(value)));
#else
		return Math::Power(2.0, value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Power2(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_exp2_ps(_mm_set_ss(value)));
#else
		return Math::Power(2.f, value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Power10(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_exp10_pd(_mm_set_sd(value)));
#else
		return Math::Power(10.0, value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Power10(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_exp10_ps(_mm_set_ss(value)));
#else
		return Math::Power(10.f, value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint64 Power10(const uint8 value) noexcept
	{
		static constexpr Array<const uint64, 20> pow10 = {
			1ull,
			10ull,
			100ull,
			1000ull,
			10000ull,
			100000ull,
			1000000ull,
			10000000ull,
			100000000ull,
			1000000000ull,
			10000000000ull,
			100000000000ull,
			1000000000000ull,
			10000000000000ull,
			100000000000000ull,
			1000000000000000ull,
			10000000000000000ull,
			100000000000000000ull,
			1000000000000000000ull,
			10000000000000000000ull
		};

		MathAssert(value < pow10.GetSize());
		return pow10[Math::Min(value, (uint8)(pow10.GetSize() - 1))];
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Exponential(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_exp_pd(_mm_set_sd(value)));
#else
		return Math::Power(Math::Constantsd::e, value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Exponential(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_exp_ps(_mm_set_ss(value)));
#else
		return Math::Power(Math::Constantsf::e, value);
#endif
	}
}
POP_MSVC_WARNINGS

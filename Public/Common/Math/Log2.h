#pragma once

#include <Common/Math/Select.h>
#include <Common/Math/MathAssert.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/Math/MathAssert.h>
#include <Common/Platform/IsConstantEvaluated.h>
#include <Common/Platform/CompilerWarnings.h>

#if USE_SVML
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace ngine::Math
{
	PUSH_MSVC_WARNINGS
	DISABLE_MSVC_WARNINGS(4714) // function 'unsigned int __vectorcall ngine::Math::Log2(unsigned int)' marked as __forceinline not inlined

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Log2(const float value) noexcept
	{
#if USE_SVML
		return _mm_cvtss_f32(_mm_log2_ps(_mm_set_ss(value)));
#else
		return ::log2f(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Log2(const double value) noexcept
	{
#if USE_SVML
		return _mm_cvtsd_f64(_mm_log2_pd(_mm_set_sd(value)));
#else
		return ::log2(value);
#endif
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr uint8 Log2(const uint8 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			return (value <= 1) ? 0 : 1 + Log2(uint8(value >> 1));
		}
		else
		{
			MathExpect(value != 0);

#if COMPILER_MSVC
			unsigned long result;
			_BitScanReverse(&result, value);
			return (uint8)result;
#elif COMPILER_CLANG || COMPILER_GCC
			return (uint8)(31u - __builtin_clz(value));
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr uint16 Log2(const uint16 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			return (value <= 1) ? 0 : 1 + Log2(uint16(value >> 1));
		}
		else
		{
			MathExpect(value != 0);

#if COMPILER_MSVC
			unsigned long result;
			_BitScanReverse(&result, value);
			return (uint16)result;
#elif COMPILER_CLANG || COMPILER_GCC
			return (uint16)(31u - __builtin_clz(value));
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr uint32 Log2(const uint32 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			return (value <= 1) ? 0 : 1 + Log2(uint32(value >> 1));
		}
		else
		{
			MathExpect(value != 0);

#if COMPILER_MSVC
			unsigned long result;
			_BitScanReverse(&result, value);
			return (uint32)result;
#elif COMPILER_CLANG || COMPILER_GCC
			return (uint32)(31u - __builtin_clz(value));
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr uint64 Log2(const uint64 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			return (value <= 1) ? 0 : 1 + Log2(uint64(value >> 1));
		}
		else
		{
			MathExpect(value != 0);

#if COMPILER_MSVC
			unsigned long result;
			_BitScanReverse64(&result, value);
			return (uint64)result;
#elif COMPILER_CLANG || COMPILER_GCC
			return (uint64)(63ull - __builtin_clzll(value));
#endif
		}
	}
	POP_MSVC_WARNINGS
}

#pragma once

#if USE_SSE
#include <emmintrin.h>
#endif

#if USE_AVX
#include <immintrin.h>
#endif

#if USE_NEON
#if PLATFORM_WINDOWS && PLATFORM_64BIT
#include <arm64_neon.h>
#else
#include <arm_neon.h>
#endif
#endif

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

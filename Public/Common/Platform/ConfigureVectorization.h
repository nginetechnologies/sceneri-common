#pragma once

#ifdef __SSE__
#	define USE_SSE 1
#else
#	define USE_SSE 0
#endif

#ifdef __SSE2__
#	define USE_SSE2 1
#else
#	define USE_SSE2 0
#endif

#ifdef __SSE3__
#	define USE_SSE3 1
#else
#	define USE_SSE3 0
#endif

#ifdef __SSSE3__
#	define USE_SSSE3 1
#else
#	define USE_SSSE3 0
#endif

#ifdef __SSE4_1__
#	define USE_SSE4_1 1
#else
#	define USE_SSE4_1 0
#endif

#ifdef __SSE4_2__
#	define USE_SSE4_2 1
#else
#	define USE_SSE4_2 0
#endif

#ifdef __AVX__
#	define USE_AVX 1
#else
#	define USE_AVX 0
#endif

#ifdef __AVX2__
#	define USE_AVX2 1
#else
#	define USE_AVX2 0
#endif

#ifdef __AVX512F__
#	define USE_AVX512 1
#else
#	define USE_AVX512 0
#endif

#ifdef __ARM_NEON__
#	define USE_NEON 1
#else
#	define USE_NEON 0
#endif

#ifdef __wasm_simd128__
#	define USE_WASM_SIMD128 1
#else
#	define USE_WASM_SIMD128 0
#endif

// Temporary while migrating
static_assert(USE_SSE == USE_SSE_OLD);
static_assert(USE_SSE2 == USE_SSE2_OLD);
static_assert(USE_SSE3 == USE_SSE3_OLD);
static_assert(USE_SSSE3 == USE_SSSE3_OLD);
static_assert(USE_SSE4_1 == USE_SSE4_1_OLD);
static_assert(USE_AVX == USE_AVX_OLD);
static_assert(USE_AVX2 == USE_AVX2_OLD);
static_assert(USE_AVX512 == USE_AVX512_OLD);
static_assert(USE_NEON == USE_NEON_OLD);
static_assert(USE_WASM_SIMD128 == USE_WASM_SIMD128_OLD);

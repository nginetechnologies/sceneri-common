#pragma once

#if COMPILER_MSVC
#define LIKELY(condition) (condition)
#elif COMPILER_CLANG || COMPILER_GCC
#define LIKELY(condition) __builtin_expect((condition), 1)
#endif

#if COMPILER_MSVC
#define UNLIKELY(condition) (condition)
#elif COMPILER_CLANG || COMPILER_GCC
#define UNLIKELY(condition) __builtin_expect((condition), 0)
#endif

#if COMPILER_MSVC
#define UNLIKELY_ERROR(condition) UNLIKELY((condition))
#elif COMPILER_GCC
#define UNLIKELY_ERROR(condition) __builtin_expect_with_probability((condition), 0, 0.99)
#elif COMPILER_CLANG
#define UNLIKELY_ERROR(condition) UNLIKELY((condition))
#endif

#pragma once

#if COMPILER_MSVC
#define NO_INLINE __declspec(noinline)
#elif COMPILER_CLANG || COMPILER_GCC
#define NO_INLINE __attribute__((noinline))
#endif

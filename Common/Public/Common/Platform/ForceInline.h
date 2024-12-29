#pragma once

#if COMPILER_MSVC
#define FORCE_INLINE __forceinline
#elif COMPILER_GCC || COMPILER_CLANG
#define FORCE_INLINE __attribute__((always_inline)) inline
#endif

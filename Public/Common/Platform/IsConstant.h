#pragma once

#if COMPILER_MSVC
#define IS_CONSTANT(value) false
#elif COMPILER_GCC
#define IS_CONSTANT(value) __builtin_constant_p(value)
#elif COMPILER_CLANG
#define IS_CONSTANT(value) (__builtin_constant_p(value), 0)
#endif

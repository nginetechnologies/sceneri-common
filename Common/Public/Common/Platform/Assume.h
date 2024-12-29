#pragma once

#if COMPILER_MSVC
#define ASSUME(condition) __assume((condition))
#elif COMPILER_CLANG
#define ASSUME(condition) __builtin_assume((condition))
#elif COMPILER_GCC 
#define ASSUME(condition) if (!(condition)) __builtin_unreachable();
#endif

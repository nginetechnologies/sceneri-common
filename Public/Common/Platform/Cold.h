#pragma once

#if COMPILER_CLANG || COMPILER_GCC
#define COLD_FUNCTION __attribute__((cold))
#else
#define COLD_FUNCTION
#endif

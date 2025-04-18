#pragma once

#if COMPILER_MSVC
#define UNREACHABLE __assume(0)
#elif COMPILER_CLANG || COMPILER_GCC
#define UNREACHABLE __builtin_unreachable();
#endif

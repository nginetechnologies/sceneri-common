#pragma once

#if COMPILER_MSVC
#define NORETURN __declspec(noreturn)
#elif COMPILER_CLANG || COMPILER_GCC
#define NORETURN __attribute__((__noreturn__))
#endif

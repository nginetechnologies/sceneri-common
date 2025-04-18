#pragma once

#if COMPILER_CLANG || COMPILER_GCC
//! Indicates to the compiler and linker that the symbol is used and should not be warned or stripped out.
#define USED __attribute__((used))
#else
#define USED
#endif

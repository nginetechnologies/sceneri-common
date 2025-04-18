#pragma once

#if COMPILER_MSVC
#define TRIVIAL_ABI
#elif COMPILER_GCC
#define TRIVIAL_ABI
#elif COMPILER_CLANG
#define TRIVIAL_ABI [[clang::trivial_abi]]
#endif

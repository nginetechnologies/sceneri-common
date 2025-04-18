#pragma once

#if COMPILER_MSVC
#define INTERNAL_LINKAGE
#elif COMPILER_CLANG || COMPILER_GCC
#define INTERNAL_LINKAGE __attribute__((internal_linkage))
#endif

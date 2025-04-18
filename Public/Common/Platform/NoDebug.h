#pragma once

#if COMPILER_CLANG
#define NO_DEBUG __attribute__((__nodebug__)) //[[clang::nodebug]]
#else
#define NO_DEBUG
#endif

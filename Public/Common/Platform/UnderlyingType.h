#pragma once

#if COMPILER_MSVC
#define UNDERLYING_TYPE(type) __underlying_type(type)
#elif COMPILER_CLANG || COMPILER_GCC
#define UNDERLYING_TYPE(type) __underlying_type(type)
#endif

#pragma once

#include <Common/Platform/CppVersion.h>

#if CPP_VERSION >= 20

#if COMPILER_MSVC
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif COMPILER_GCC || COMPILER_CLANG
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#else // <= C++17

#if COMPILER_MSVC
#pragma warning(disable : 4848) // support for standard attribute 'no_unique_address' in C++17 and earlier is a vendor extension
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif COMPILER_CLANG || COMPILER_GCC
#define NO_UNIQUE_ADDRESS
#endif

#endif

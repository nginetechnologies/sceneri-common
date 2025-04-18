#pragma once

#if defined(__clang__)
#   define COMPILER_CLANG 1
#   if defined(PLATFORM_WINDOWS)
#       define COMPILER_CLANG_WINDOWS 1
#   endif
#elif defined(__GNUC__)
#   define COMPILER_GCC 1
#elif defined(_MSC_VER)
#   define COMPILER_MSVC 1
#else
#   error "Unknown Compiler"
#endif

#ifndef COMPILER_CLANG
#   define COMPILER_CLANG 0
#endif

#ifndef COMPILER_CLANG_WINDOWS
#   define COMPILER_CLANG_WINDOWS 0
#endif

#ifndef COMPILER_GCC
#   define COMPILER_GCC 0
#endif

#ifndef COMPILER_MSVC
#   define COMPILER_MSVC 0
#endif

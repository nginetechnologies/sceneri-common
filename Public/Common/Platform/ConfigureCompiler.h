#pragma once

#if defined(__clang__)
#   define COMPILER_CLANG 1
#   if PLATFORM_WINDOWS
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

// Temporary while porting
static_assert(COMPILER_MSVC == COMPILER_MSVC_OLD);
static_assert(COMPILER_GCC == COMPILER_GCC_OLD);
static_assert(COMPILER_CLANG == COMPILER_CLANG_OLD);
static_assert(COMPILER_CLANG_WINDOWS == COMPILER_CLANG_WINDOWS_OLD);

#pragma once

#if COMPILER_MSVC
#define PUSH_MSVC_WARNINGS __pragma(warning(push))
#define PUSH_MSVC_WARNINGS_TO_LEVEL(level) __pragma(warning(push, level))
#define DISABLE_MSVC_WARNINGS(warnings) __pragma(warning(disable : warnings))
#define POP_MSVC_WARNINGS __pragma(warning(pop))
#else
#define PUSH_MSVC_WARNINGS
#define DISABLE_MSVC_WARNINGS(warnings)
#define PUSH_MSVC_WARNINGS_TO_LEVEL(level)
#define POP_MSVC_WARNINGS
#endif

#if COMPILER_CLANG
#define PUSH_CLANG_WARNINGS _Pragma("clang diagnostic push")
#define POP_CLANG_WARNINGS _Pragma("clang diagnostic pop")

#define CLANG_PRAGMA(x) _Pragma(#x)
#define DISABLE_CLANG_WARNING(x) CLANG_PRAGMA(clang diagnostic ignored x)
#else
#define PUSH_CLANG_WARNINGS
#define DISABLE_CLANG_WARNING(x)
#define POP_CLANG_WARNINGS
#endif

#if COMPILER_GCC
#define PUSH_GCC_WARNINGS _Pragma("GCC diagnostic push")
#define POP_GCC_WARNINGS _Pragma("GCC diagnostic pop")

#define GCC_PRAGMA(x) _Pragma(#x)
#define DISABLE_GCC_WARNING(x) GCC_PRAGMA(GCC diagnostic ignored x)
#else
#define PUSH_GCC_WARNINGS
#define DISABLE_GCC_WARNING(x)
#define POP_GCC_WARNINGS
#endif

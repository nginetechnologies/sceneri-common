#pragma once

#if COMPILER_CLANG || COMPILER_GCC
#   if defined(__x86_64__) || defined(__i386__)
#       define PLATFORM_X86 1
#   elif defined(__arm__) || defined(__aarch64__)
#       define PLATFORM_ARM 1
#   elif (__EMSCRIPTEN__)
#       define PLATFORM_WEB 1
#   else
#       error unhandled target architecture
#   endif

#   if defined(__x86_64__) || defined(__aarch64__)
#       define PLATFORM_64BIT 1
#   elif defined(__i386__) || defined(__arm__)
#       define PLATFORM_32BIT 1
#   elif (__EMSCRIPTEN__)
#       define PLATFORM_32BIT 1
#   else
#       error unhandled platform bit count
#   endif
#elif COMPILER_MSVC
#   if defined(_M_AMD64) || defined(_M_IX86)
#       define PLATFORM_X86 1
#   elif defined(_M_ARM) || defined(_M_ARM64)
#       define PLATFORM_ARM 1
#   else
#       error unhandled target architecture
#   endif

#   if defined(_M_AMD64) || defined(_M_ARM64)
#       define PLATFORM_64BIT 1
#   elif defined(_M_IX86) || defined(_M_ARM)
#       define PLATFORM_32BIT 1
#   else
#       error unhandled platform bit count
#   endif
#endif


#ifndef PLATFORM_X86
#   define PLATFORM_X86 0
#endif

#ifndef PLATFORM_ARM
#   define PLATFORM_ARM 0
#endif

#ifndef PLATFORM_WEB
#   define PLATFORM_WEB 0
#endif

#ifndef PLATFORM_64BIT
#   define PLATFORM_64BIT 0
#endif

#ifndef PLATFORM_32BIT
#   define PLATFORM_32BIT 0
#endif

#if PLATFORM_64BIT && !PLATFORM_X86
#	define _AMD64_
#endif

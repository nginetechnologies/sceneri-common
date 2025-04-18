#pragma once

#if COMPILER_CLANG
#if __has_cpp_attribute(clang::lifetimebound) && !PLATFORM_LINUX
#define LIFETIME_BOUND [[clang::lifetimebound]]
#else
#define LIFETIME_BOUND
#endif
#elif COMPILER_MSVC && __has_cpp_attribute(msvc::lifetimebound)
#define LIFETIME_BOUND [[msvc::lifetimebound]]
#else
#define LIFETIME_BOUND
#endif

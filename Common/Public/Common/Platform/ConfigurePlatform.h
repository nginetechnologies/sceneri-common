#pragma once

#if defined(_WINDOWS) || defined(_WIN32)
#   define PLATFORM_NAME "Windows"
#   define PLATFORM_DESKTOP 1
#   define PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#   include <TargetConditionals.h>
#   define PLATFORM_APPLE 1
#   define PLATFORM_POSIX 1
#   if TARGET_OS_MAC == 1
#       define PLATFORM_NAME "MacOS"
#       define PLATFORM_DESKTOP 1
#       define PLATFORM_APPLE_MACOS 1
#   elif TARGET_OS_IOS == 1
#       if TARGET_OS_MACCATALYST == 1
#           define PLATFORM_NAME "macCatalyst"
#           define PLATFORM_DESKTOP 1
#           define PLATFORM_APPLE_MACCATALYST 1
#       else
#           define PLATFORM_NAME "iOS"
#           define PLATFORM_MOBILE 1
#           define PLATFORM_APPLE_IOS 1
#       endif
#   elif TARGET_OS_VISION == 1
#       define PLATFORM_NAME "visionOS"
#       define PLATFORM_SPATIAL 1
#       define PLATFORM_APPLE_VISIONOS 1
#   elif TARGET_OS_WATCH == 1
#       define PLATFORM_NAME "watchOS"
#       define PLATFORM_WATCH 1
#       define PLATFORM_APPLE_WATCH 1
#   endif
#elif defined (ANDROID)
#   define PLATFORM_NAME "Android"
#   define PLATFORM_MOBILE 1
#   define PLATFORM_ANDROID 1
#   define PLATFORM_POSIX 1
#elif defined(__linux)
#   define PLATFORM_NAME "Linux"
#   define PLATFORM_DESKTOP 1
#   define PLATFORM_LINUX 1
#   define PLATFORM_POSIX 1
#elif defined(__EMSCRIPTEN__)
#   define PLATFORM_NAME "Web"
#   define PLATFORM_WEB 1
#   define PLATFORM_EMSCRIPTEN 1
#   define PLATFORM_POSIX 1
#elif defined(__wasm__) || defined(__wasi__)
#   define PLATFORM_NAME "Web"
#   define PLATFORM_WEB 1
#   define PLATFORM_POSIX 1
#else
#   error "PLATFORM UNDEFINED"
#endif

#ifndef PLATFORM_NAME
#   define PLATFORM_NAME "UNKNOWN"
#endif

#ifndef PLATFORM_DESKTOP
#   define PLATFORM_DESKTOP 0
#endif

#ifndef PLATFORM_MOBILE
#   define PLATFORM_MOBILE 0
#endif

#ifndef PLATFORM_SPATIAL
#   define PLATFORM_SPATIAL 0
#endif

#ifndef PLATFORM_WEB
#   define PLATFORM_WEB 0
#endif

#ifndef PLATFORM_WINDOWS
#   define PLATFORM_WINDOWS 0
#endif

#ifndef PLATFORM_APPLE
#   define PLATFORM_APPLE 0
#endif

#ifndef PLATFORM_POSIX
#   define PLATFORM_POSIX 0
#endif

#ifndef PLATFORM_APPLE_MACOS
#   define PLATFORM_APPLE_MACOS 0
#endif

#ifndef PLATFORM_APPLE_MACOS
#   define PLATFORM_APPLE_MACOS 0
#endif

#ifndef PLATFORM_APPLE_MACCATALYST
#   define PLATFORM_APPLE_MACCATALYST 0
#endif

#ifndef PLATFORM_APPLE_IOS
#   define PLATFORM_APPLE_IOS 0
#endif

#ifndef PLATFORM_APPLE_VISIONOS
#   define PLATFORM_APPLE_VISIONOS 0
#endif

#ifndef PLATFORM_APPLE_WATCH
#   define PLATFORM_APPLE_WATCH 0
#endif

#ifndef PLATFORM_ANDROID
#   define PLATFORM_ANDROID 0
#endif

#ifndef PLATFORM_LINUX
#   define PLATFORM_LINUX 0
#endif

#ifndef PLATFORM_EMSCRIPTEN
#   define PLATFORM_EMSCRIPTEN 0
#endif

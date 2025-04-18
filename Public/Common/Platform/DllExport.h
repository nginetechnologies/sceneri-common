#pragma once

#if PLATFORM_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif PLATFORM_POSIX
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT __attribute__((visibility("default")))
#endif

/* ----------------------------------------------------------------------------
Copyright (c) 2018-2023, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include <Common/Platform/CompilerWarnings.h>

PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wcast-qual")
DISABLE_CLANG_WARNING("-Wcast-align")
DISABLE_CLANG_WARNING("-Wunused-parameter")

PUSH_GCC_WARNINGS
DISABLE_GCC_WARNING("-Wunused-parameter")

// Select the implementation of the primitives
// depending on the OS.

#if defined(_WIN32)
#pragma warning(disable : 4574) // __has_builtin' is defined to be '0'
#include "Common/3rdparty/mimalloc/prim/windows/prim.h"       // VirtualAlloc (Windows)

#elif defined(__EMSCRIPTEN__)
#include "Common/3rdparty/mimalloc/prim/emscripten/prim.h" // emmalloc_*, + pthread support

#elif defined(__APPLE__)
#include "Common/3rdparty/mimalloc/prim/osx/prim.h" // macOSX (actually defers to mmap in unix/prim.c)

#elif defined(__wasi__)
#define MI_USE_SBRK
#include "Common/3rdparty/mimalloc/prim/wasi/prim.h" // memory-grow or sbrk (Wasm)

#else
#include "Common/3rdparty/mimalloc/prim/unix/prim.h" // mmap() (Linux, macOSX, BSD, Illumnos, Haiku, DragonFly, etc.)

#endif
POP_CLANG_WARNINGS
POP_GCC_WARNINGS

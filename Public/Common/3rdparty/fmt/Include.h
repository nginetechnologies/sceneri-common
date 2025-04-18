#pragma once

#include <Common/Platform/CompilerWarnings.h>

#define FMT_HEADER_ONLY 0
#define FMT_USE_WINDOWS_H 0
#define FMT_EXCEPTIONS 0
#define FMT_USE_TYPEID 0
#define FMT_CPP_LIB_FILESYSTEM 0
#define FMT_CPP_LIB_VARIANT 0
#define FMT_UNICODE 0

PUSH_MSVC_WARNINGS
DISABLE_MSVC_WARNINGS(4574)
DISABLE_MSVC_WARNINGS(4702)
DISABLE_MSVC_WARNINGS(4996)

PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Winconsistent-missing-destructor-override")
DISABLE_CLANG_WARNING("-Wdeprecated-declarations")
DISABLE_CLANG_WARNING("-Wfloat-equal")
DISABLE_CLANG_WARNING("-Wsign-conversion")
DISABLE_CLANG_WARNING("-Wmissing-noreturn")
DISABLE_CLANG_WARNING("-Wsigned-enum-bitfield")
DISABLE_CLANG_WARNING("-Wdeprecated")
DISABLE_CLANG_WARNING("-Wshorten-64-to-32")
DISABLE_CLANG_WARNING("-Winfinity-disabled")
DISABLE_CLANG_WARNING("-Wnan-infinity-disabled")

#if 0//CONTINUOUS_INTEGRATION
#include "format.h"
#include "compile.h"
#else
#include "base.h"
#define FMT_COMPILE(x) x
#endif

POP_CLANG_WARNINGS
POP_MSVC_WARNINGS

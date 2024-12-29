#pragma once

#include <Common/Assert/Assert.h>
#include <Common/Platform/CompilerWarnings.h>
#include <Common/Memory/Move.h>
#include <Common/Memory/Forward.h>

PUSH_MSVC_WARNINGS
DISABLE_MSVC_WARNINGS(4996 5054)

PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wzero-as-null-pointer-constant")
DISABLE_CLANG_WARNING("-Wswitch-enum")
DISABLE_CLANG_WARNING("-Wreserved-id-macro")
DISABLE_CLANG_WARNING("-Wunknown-pragmas")
DISABLE_CLANG_WARNING("-Wundefined-func-template")
DISABLE_CLANG_WARNING("-Wempty-body")
DISABLE_CLANG_WARNING("-Wnan-infinity-disabled")
DISABLE_CLANG_WARNING("-Wnontrivial-memcall")

PUSH_GCC_WARNINGS
DISABLE_GCC_WARNING("-Wclass-memaccess")

#if COMPILER_CLANG
#undef _MSC_VER
#endif

#define RAPIDJSON_ASSERT Assert
#define RAPIDJSON_STATIC_ASSERT(x) static_assert(x)

#if USE_SSE2
#define RAPIDJSON_SSE2 1
#endif

#if USE_SSE4_2
#define RAPIDJSON_SSE42 1
#endif

#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#define RAPIDJSON_HAS_CXX11_NOEXCEPT 1
#define RAPIDJSON_HAS_STDSTRING 0
#define RAPIDJSON_HAS_CXX11_TYPETRAITS 1
#define RAPIDJSON_HAS_CXX11_RANGE_FOR 1

namespace rapidjson
{
	using ngine::Move;
	using ngine::Forward;
}

#include <Common/3rdparty/rapidjson/document.h>
#include <Common/3rdparty/rapidjson/writer.h>
#include <Common/3rdparty/rapidjson/prettywriter.h>

#undef RAPIDJSON_ASSERT
#undef RAPIDJSON_STATIC_ASSERT
#undef RAPIDJSON_SSE2
#undef RAPIDJSON_SSE42
#undef RAPIDJSON_HAS_CXX11_RVALUE_REFS
#undef RAPIDJSON_HAS_CXX11_NOEXCEPT
#undef RAPIDJSON_HAS_STDSTRING
#undef RAPIDJSON_HAS_CXX11_TYPETRAITS
#undef RAPIDJSON_HAS_CXX11_RANGE_FOR

POP_MSVC_WARNINGS
POP_CLANG_WARNINGS
POP_GCC_WARNINGS

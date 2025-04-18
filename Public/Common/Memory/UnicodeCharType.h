#pragma once

#include <Common/Platform/CppVersion.h>

namespace ngine
{
	using UnicodeCharType = char16_t;
#define MAKE_UNICODE_LITERAL(path) u##path

#if CPP_VERSION >= 20
	using UTF8CharType = char8_t;
#define IS_UNICODE_CHAR8_UNIQUE_TYPE 1
#else
	using UTF8CharType = char;
#define IS_UNICODE_CHAR8_UNIQUE_TYPE 0
#endif
}

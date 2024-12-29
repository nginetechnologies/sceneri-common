#pragma once

#if PLATFORM_WINDOWS
#define MAKE_NATIVE_LITERAL(path) L##path
#else
#define MAKE_NATIVE_LITERAL(path) path
#endif

namespace ngine
{
#if PLATFORM_WINDOWS
	using NativeCharType = wchar_t;
#else
	using NativeCharType = char;
#endif
}

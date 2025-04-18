#pragma once

namespace ngine::IO
{
#if PLATFORM_WINDOWS
	using PathCharType = wchar_t;
#else
	using PathCharType = char;
#endif

#if PLATFORM_WINDOWS
#define MAKE_PATH_LITERAL(path) L##path
#else
#define MAKE_PATH_LITERAL(path) path
#endif
}

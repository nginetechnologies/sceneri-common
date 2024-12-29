#pragma once

#include <Common/IO/PathCharType.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::IO
{
	inline static constexpr bool CaseSensitive = !PLATFORM_WINDOWS;

#if PLATFORM_WINDOWS
	inline static constexpr PathCharType PathSeparator = MAKE_PATH_LITERAL('\\');
#else
	inline static constexpr PathCharType PathSeparator = '/';
#endif
	inline static constexpr uint16 MaximumPathLength = 1024;
}

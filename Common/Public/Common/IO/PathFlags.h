#pragma once

#include <Common/EnumFlagOperators.h>

namespace ngine::IO
{
	enum class PathFlags : uint8
	{
		CaseSensitive = 1 << 0,
		//! Whether the path can support query strings, aka ?arg=value&arg2=value2
		SupportQueries = 1 << 1,
	};
	ENUM_FLAG_OPERATORS(PathFlags);
}

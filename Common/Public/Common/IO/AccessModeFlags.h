#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/EnumFlagOperators.h>

namespace ngine::IO
{
	enum class AccessModeFlags : uint8
	{
		Read = 1 << 0,
		Write = 1 << 1,
		Append = 1 << 2,
		Binary = 1 << 3,
		ReadBinary = Read | Binary,
		WriteBinary = Write | Binary,
	};

	ENUM_FLAG_OPERATORS(AccessModeFlags);
}

#pragma once

#include <Common/EnumFlagOperators.h>

namespace ngine::Serialization
{
	enum class ContextFlags : uint8
	{
		FromDisk = 1 << 0,
		FromBuffer = 1 << 1,
		ToDisk = 1 << 2,
		ToBuffer = 1 << 3,
		UndoHistory = 1 << 4,
		Duplication = 1 << 5,
		//! True when the serialized data is intended to be written and read from the same session
		UseWithinSessionInstance = 1 << 6,
	};

	ENUM_FLAG_OPERATORS(ContextFlags)
}

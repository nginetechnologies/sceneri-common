#pragma once

#include <Common/EnumFlagOperators.h>

namespace ngine::Serialization
{
	enum class SavingFlags : uint8
	{
		//! Whether the serialized data is intended to be read by a human
		//! Implies that formatting should be applied.
		HumanReadable = 1 << 0
	};
	ENUM_FLAG_OPERATORS(SavingFlags);
}

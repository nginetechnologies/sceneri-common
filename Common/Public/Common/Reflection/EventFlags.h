#pragma once

#include <Common/EnumFlagOperators.h>

namespace ngine::Reflection
{
	enum class EventFlags : uint8
	{
		//! Whether the event should be visible and usable from UI (i.e. logic graphs)
		VisibleToUI = 1 << 0
	};
	ENUM_FLAG_OPERATORS(EventFlags);
};

#pragma once

#include "FunctionArgument.h"
#include "EventFlags.h"

#include <Common/Function/FunctionPointer.h>

namespace ngine::Reflection
{
	struct EventInfo
	{
		using GetArgumentsFunction = FunctionPointer<ArrayView<const Argument, uint8>(const EventInfo& event)>;

		constexpr EventInfo(const Guid guid, const ConstUnicodeStringView displayName, GetArgumentsFunction&& getArgumentsFunction)
			: m_guid(guid)
			, m_displayName(displayName)
			, m_getArgumentsFunction(Forward<GetArgumentsFunction>(getArgumentsFunction))
		{
		}

		Guid m_guid;
		ConstUnicodeStringView m_displayName;
		EnumFlags<EventFlags> m_flags{EventFlags::VisibleToUI};
		GetArgumentsFunction m_getArgumentsFunction;
	};
}

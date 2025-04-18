#pragma once

#include "FunctionArgument.h"
#include "FunctionFlags.h"

#include <Common/Function/FunctionPointer.h>

namespace ngine::Reflection
{
	struct ReturnType : public Argument
	{
		using Argument::Argument;
		constexpr ReturnType(Argument&& argument)
			: Argument(Forward<Argument>(argument))
		{
		}
	};

	struct FunctionInfo
	{
		using GetArgumentsFunction = FunctionPointer<ArrayView<const Argument, uint8>(const FunctionInfo& function)>;

		constexpr FunctionInfo(
			const Guid guid,
			const ConstUnicodeStringView displayName,
			const EnumFlags<FunctionFlags> flags,
			Argument&& returnType,
			GetArgumentsFunction&& getArgumentsFunction
		)
			: m_guid(guid)
			, m_displayName(displayName)
			, m_flags(flags)
			, m_returnType(Forward<Argument>(returnType))
			, m_getArgumentsFunction(Forward<GetArgumentsFunction>(getArgumentsFunction))
		{
		}

		Guid m_guid;
		ConstUnicodeStringView m_displayName;
		EnumFlags<FunctionFlags> m_flags;
		ReturnType m_returnType;
		GetArgumentsFunction m_getArgumentsFunction;
	};
}

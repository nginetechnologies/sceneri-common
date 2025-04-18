#pragma once

#include "TypeDefinition.h"

#include <Common/EnumFlags.h>
#include <Common/EnumFlagOperators.h>

namespace ngine::Reflection
{
	enum class ArgumentFlags : uint8
	{
		//! Whether the argument should be treated as 'self'
		//! This can only be used once per function, and indicates that this function should act as an extension of another type
		IsSelf = 1 << 0,
		//! Whether this argument has a default value, meaning that it does not need to be provided
		//! Currently only supported if the default value is the default constructed argument type
		IsOptional = 1 << 1,
		//! Whether the argument should be hidden from UI aka logic graphs
		//! Only possible if IsOptional is also set.
		HideFromUI = 1 << 2,
	};
	ENUM_FLAG_OPERATORS(ArgumentFlags)

	struct Argument
	{
		constexpr Argument()
		{
		}
		constexpr Argument(const Guid guid, const ConstUnicodeStringView name, const ArgumentFlags flags = {}, const TypeDefinition type = {})
			: m_guid(guid)
			, m_name(name)
			, m_flags(flags)
			, m_type(type)
		{
		}

		Guid m_guid;
		ConstUnicodeStringView m_name;
		EnumFlags<ArgumentFlags> m_flags;
		TypeDefinition m_type;
	};
};

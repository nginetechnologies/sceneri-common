#pragma once

#include "PropertyFlags.h"

#include <Common/EnumFlags.h>
#include <Common/Guid.h>
#include <Common/Reflection/TypeDefinition.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Platform/Pure.h>

namespace ngine::Reflection
{
	struct PropertyInfo
	{
		static constexpr TypeDefinition EmptyTypeDefinition{};

		[[nodiscard]] PURE_STATICS bool operator==(const PropertyInfo& other) const
		{
			return m_guid == other.m_guid && m_typeGuid == other.m_typeGuid && m_typeDefinition == other.m_typeDefinition;
		}
		[[nodiscard]] PURE_STATICS bool operator!=(const PropertyInfo& other) const
		{
			return !operator==(other);
		}

		ConstUnicodeStringView m_displayName;
		ConstStringView m_name;
		Guid m_guid;
		ConstUnicodeStringView m_categoryDisplayName;
		EnumFlags<PropertyFlags> m_flags;
		Guid m_typeGuid;
		TypeDefinition m_typeDefinition = EmptyTypeDefinition;
	};
};

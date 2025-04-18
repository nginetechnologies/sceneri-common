#pragma once

#include <Common/Reflection/PropertyInfo.h>
#include <Common/Memory/Containers/String.h>

namespace ngine::Reflection
{
	struct Registry;

	struct DynamicPropertyInfo : public PropertyInfo
	{
		DynamicPropertyInfo() = default;
		DynamicPropertyInfo(PropertyInfo&& propertyInfo)
			: PropertyInfo(Forward<PropertyInfo>(propertyInfo))
		{
			m_storedDisplayName = propertyInfo.m_displayName;
			m_displayName = m_storedDisplayName;
			m_storedName = propertyInfo.m_name;
			m_name = m_storedName;
			m_storedCategoryDisplayName = propertyInfo.m_categoryDisplayName;
			m_categoryDisplayName = m_storedCategoryDisplayName;
		}
		using PropertyInfo::PropertyInfo;
		using PropertyInfo::operator=;

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer reader) const;

		UnicodeString m_storedDisplayName;
		String m_storedName;
		UnicodeString m_storedCategoryDisplayName;

		uint32 m_offsetFromOwner = 0;
		uint32 m_offsetToNextProperty = 0;
	};
}

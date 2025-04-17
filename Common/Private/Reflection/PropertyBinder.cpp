#include "Common/Reflection/PropertyBinder.h"

#include <Common/System/Query.h>
#include <Common/Memory/Move.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>

#include <Common/Reflection/Registry.inl>

namespace ngine::Reflection
{
	PropertyBinder::PropertyBinder()
	{
	}

	PropertyBinder::PropertyBinder(const PropertyBinder& other) noexcept
		: m_properties(other.m_properties)
	{
	}

	PropertyBinder::PropertyBinder(PropertyBinder&& other) noexcept
		: m_properties(Move(other.m_properties))
	{
	}

	PropertyBinder::~PropertyBinder()
	{
	}

	void PropertyBinder::SetProperty(String&& key, Any&& value)
	{
		[[maybe_unused]] PropertyMap::iterator propertyIt = m_properties.EmplaceOrAssign(Forward<String>(key), Forward<Any>(value));
	}

	ConstAnyView PropertyBinder::GetProperty(ConstStringView key) const
	{
		auto propertyIt = m_properties.Find(key);
		if (propertyIt != m_properties.end())
		{
			return propertyIt->second;
		}
		return {};
	}

	const PropertyBinder::PropertyMap& PropertyBinder::GetProperties() const
	{
		return m_properties;
	}

	void PropertyBinder::Reset()
	{
		m_properties.Clear();
	}

	void PropertyBinder::DeserializeCustomData(const Optional<Serialization::Reader> pReader)
	{
		if (pReader)
		{
			pReader->Serialize("values", m_properties);
		}
	}

	bool PropertyBinder::SerializeCustomData(Serialization::Writer writer) const
	{
		writer.Serialize("values", m_properties);
		return true;
	}

	bool PropertyBinder::operator==(const PropertyBinder& other) const
	{
		if (m_properties.GetSize() != other.m_properties.GetSize())
		{
			return false;
		}

		const PropertyMap& otherProperties = other.GetProperties();
		for (auto propertyIt : m_properties)
		{
			auto otherPropertyIt = otherProperties.Find(propertyIt.first);
			if (otherPropertyIt == otherProperties.end())
			{
				return false;
			}

			if (propertyIt.second != otherPropertyIt->second)
			{
				return false;
			}
		}
		return true;
	}

	PropertyBinder& PropertyBinder::operator=(const PropertyBinder& other) noexcept
	{
		if (this != &other)
		{
			m_properties = other.m_properties;
		}
		return *this;
	}

	PropertyBinder& PropertyBinder::operator=(PropertyBinder&& other) noexcept
	{
		if (this != &other)
		{
			m_properties = Move(other.m_properties);
		}
		return *this;
	}

	[[maybe_unused]] const bool wasDynamicPropertyBinderTypeRegistered = Registry::RegisterType<PropertyBinder>();
}

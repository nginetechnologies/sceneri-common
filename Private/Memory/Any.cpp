#include "Memory/Any.h"

#include <Common/Reflection/Registry.h>
#include <Common/Serialization/Guid.h>
#include <Common/System/Query.h>

namespace ngine
{
	template<typename AllocatorType_>
	bool TAny<AllocatorType_>::Serialize(Serialization::Writer serializer) const
	{
		if (!IsValid())
		{
			return false;
		}

		Reflection::Registry& registry = System::Get<Reflection::Registry>();
		if (!serializer.Serialize("type", registry.FindTypeDefinitionGuid(m_typeDefinition)))
		{
			return false;
		}

		Serialization::Value value;
		Serialization::Writer valueSerializer(value, serializer.GetData());
		const bool result = m_typeDefinition.SerializeStoredObject(m_storage.GetData(), valueSerializer);
		serializer.AddMember("value", Move(value));
		return result;
	}

	template<typename AllocatorType_>
	bool TAny<AllocatorType_>::Serialize(const Serialization::Reader serializer)
	{
		if (const Optional<Reflection::TypeDefinition> typeDefinition = serializer.Read<Reflection::TypeDefinition>("type"))
		{
			*this = TAny(Memory::Uninitialized, *typeDefinition);
			return typeDefinition->SerializeConstructStoredObject(m_storage.GetData(), *serializer.FindSerializer("value"));
		}
		return false;
	}

	bool AnyView::Serialize(Serialization::Writer serializer) const
	{
		if (!IsValid())
		{
			return false;
		}

		Reflection::Registry& registry = System::Get<Reflection::Registry>();
		if (!serializer.Serialize("type", registry.FindTypeDefinitionGuid(m_typeDefinition)))
		{
			return false;
		}

		Serialization::Value value;
		Serialization::Writer valueSerializer(value, serializer.GetData());
		const bool result = m_typeDefinition.SerializeStoredObject(m_pData, valueSerializer);
		serializer.AddMember("value", Move(value));
		return result;
	}

	template struct TAny<Memory::DynamicAllocator<ByteType, uint32>>;
}

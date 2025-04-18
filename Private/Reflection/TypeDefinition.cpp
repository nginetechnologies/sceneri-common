#include <Common/Reflection/TypeDefinition.h>

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Serialization/Guid.h>
#include <Common/Reflection/Registry.h>
#include <Common/Compression/Compressor.h>
#include <Common/TypeTraits/TypeName.h>
#include <Common/System/Query.h>

namespace ngine::Reflection
{
	uint32 TypeDefinition::GetSize() const
	{
		uint32 result;
		OperationArgument argument = {*this};
		argument.m_typeArgs[0] = &result;
		m_manager(Operation::GetTypeSize, argument);
		return result;
	}

	uint16 TypeDefinition::GetAlignment() const
	{
		uint16 result;
		OperationArgument argument = {*this};
		argument.m_typeArgs[0] = &result;
		m_manager(Operation::GetTypeAlignment, argument);
		return result;
	}

	ConstUnicodeStringView TypeDefinition::GetTypeName() const
	{
		ConstUnicodeStringView result;
		OperationArgument argument = {*this};
		argument.m_typeArgs[0] = &result;
		m_manager(Operation::GetTypeName, argument);
		return result;
	}

	bool TypeDefinition::IsTriviallyCopyable() const
	{
		bool result;
		OperationArgument argument = {*this};
		argument.m_typeArgs[0] = &result;
		m_manager(Operation::IsTriviallyCopyable, argument);
		return result;
	}

	TypeDefinition TypeDefinition::GetPointerTypeDefinition() const
	{
		OperationArgument argument = {*this};
		m_manager(Operation::GetPointerTypeDefinition, argument);
		return TypeDefinition(reinterpret_cast<ManagerFunction>(argument.m_typeArgs[0]), m_pUserData);
	}

	TypeDefinition TypeDefinition::GetValueTypeDefinition() const
	{
		OperationArgument argument = {*this};
		m_manager(Operation::GetValueTypeDefinition, argument);
		return TypeDefinition(reinterpret_cast<ManagerFunction>(argument.m_typeArgs[0]), m_pUserData);
	}

	bool TypeDefinition::AreReferencesEqual(const ConstReferenceType a, const ConstReferenceType b) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = const_cast<ReferenceType>(a);
		argument.m_objectReferenceArgs.m_args[0] = const_cast<ReferenceType>(b);
		m_manager(Operation::AreReferencesEqual, argument);
		return argument.m_objectReferenceArgs.m_args[0] != nullptr;
	}

	ByteView TypeDefinition::GetByteViewAtAddress(const ReferenceType address) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = address;
		ByteView result;
		argument.m_objectReferenceArgs.m_args[0] = &result;
		m_manager(Operation::GetReferenceByteView, argument);
		return result;
	}

	void TypeDefinition::DefaultConstructAt(void* pAddress) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = pAddress;
		m_manager(Operation::ReferencePlacementNewDefaultConstruct, argument);
	}
	void TypeDefinition::CopyConstructAt(void* pAddress, const void* pSourceAddress) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = pAddress;
		argument.m_objectReferenceArgs.m_args[0] = const_cast<void*>(pSourceAddress);
		m_manager(Operation::ReferencePlacementNewCopyConstruct, argument);
	}
	void TypeDefinition::MoveConstructAt(void* pAddress, void* pSourceAddress) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = pAddress;
		argument.m_objectReferenceArgs.m_args[0] = pSourceAddress;
		m_manager(Operation::ReferencePlacementNewMoveConstruct, argument);
	}

	void* TypeDefinition::GetAlignedObject(void* pUnalignedAddress) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = pUnalignedAddress;
		m_manager(Operation::GetStoredObjectReference, argument);
		return argument.m_objectReferenceArgs.pObject;
	}
	void TypeDefinition::DestroyUnalignedObject(const void* pUnalignedAddress) const
	{
		OperationArgument argument = {*this};
		argument.m_objectReferenceArgs.pObject = const_cast<void*>(pUnalignedAddress);
		m_manager(Operation::StoredObjectDestroy, argument);
	}

	bool TypeDefinition::AreStoredObjectsEqual(const ConstStoredType a, const ConstStoredType b) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = const_cast<StoredType>(a);
		argument.m_storedObjectArgs.m_args[0] = const_cast<StoredType>(b);
		m_manager(Operation::AreStoredObjectsEqual, argument);
		return argument.m_storedObjectArgs.pObject != nullptr;
	}

	uint32 TypeDefinition::TryDefaultConstructStoredObject(const StoredType target, uint32 targetCapacity) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = target;
		argument.m_storedObjectArgs.m_args[0] = &targetCapacity;
		m_manager(Operation::StoredObjectPlacementNewDefaultConstruct, argument);
		return targetCapacity;
	}

	uint32 TypeDefinition::TryCopyConstructStoredObject(const StoredType target, uint32 targetCapacity, const ConstStoredType source) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = target;
		argument.m_storedObjectArgs.m_args[0] = &targetCapacity;
		argument.m_storedObjectArgs.m_args[1] = const_cast<StoredType>(source);
		m_manager(Operation::StoredObjectPlacementNewCopyConstruct, argument);
		return targetCapacity;
	}

	uint32 TypeDefinition::TryMoveConstructStoredObject(const StoredType target, uint32 targetCapacity, const StoredType source) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = target;
		argument.m_storedObjectArgs.m_args[0] = &targetCapacity;
		argument.m_storedObjectArgs.m_args[1] = source;
		m_manager(Operation::StoredObjectPlacementNewMoveConstruct, argument);
		return targetCapacity;
	}

	bool TypeDefinition::Serialize(const Serialization::Reader reader)
	{
		if (reader.IsString())
		{
			const Guid guid = *reader.ReadInPlace<Guid>();
			Reflection::Registry& registry = System::Get<Reflection::Registry>();
			if (Optional<const TypeDefinition*> pDefinition = registry.FindTypeDefinition(guid))
			{
				*this = *pDefinition;
				return true;
			}
		}
		else
		{
			Assert(reader.IsObject());
			const Guid guid = reader.ReadWithDefaultValue<Guid>("guid", Guid{});
			if (guid.IsValid())
			{
				Assert(reader.ReadWithDefaultValue<bool>("pointer", false), "No other flags supported at the moment");

				Reflection::Registry& registry = System::Get<Reflection::Registry>();
				if (Optional<const TypeDefinition*> pDefinition = registry.FindTypeDefinition(guid))
				{
					*this = pDefinition->GetPointerTypeDefinition();
					return true;
				}
			}
		}
		return false;
	}

	bool TypeDefinition::Serialize(Serialization::Writer writer) const
	{
		Reflection::Registry& registry = System::Get<Reflection::Registry>();
		Guid typeGuid = registry.FindTypeDefinitionGuid(*this);
		if (typeGuid.IsValid())
		{
			return writer.SerializeInPlace(typeGuid);
		}

		// Support serializing pointers
		const TypeDefinition valueTypeDefinition = GetValueTypeDefinition();
		if (valueTypeDefinition != *this)
		{
			typeGuid = registry.FindTypeDefinitionGuid(valueTypeDefinition);
			if (typeGuid.IsValid())
			{
				writer.Serialize("guid", typeGuid);
				writer.Serialize("pointer", true);
				return true;
			}
		}

		return false;
	}

	bool TypeDefinition::SerializeStoredObject(const ConstStoredType target, Serialization::Writer& serializer) const
	{
		if (!IsValid())
		{
			return true;
		}

		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = const_cast<StoredType>(target);
		argument.m_storedObjectArgs.m_args[0] = &serializer;
		m_manager(Operation::SerializeStoredObject, argument);
		return argument.m_storedObjectArgs.pObject != nullptr;
	}

	bool TypeDefinition::SerializeStoredObject(const StoredType target, const Serialization::Reader& serializer) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = target;
		argument.m_storedObjectArgs.m_args[0] = &const_cast<Serialization::Reader&>(serializer);
		m_manager(Operation::DeserializeStoredObject, argument);
		return argument.m_storedObjectArgs.pObject != nullptr;
	}

	bool TypeDefinition::SerializeConstructStoredObject(const StoredType target, const Serialization::Reader& serializer) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = target;
		argument.m_storedObjectArgs.m_args[0] = &const_cast<Serialization::Reader&>(serializer);
		m_manager(Operation::DeserializeConstructStoredObject, argument);
		return argument.m_storedObjectArgs.pObject != nullptr;
	}

	bool TypeDefinition::HasDynamicCompressedData(EnumFlags<Reflection::PropertyFlags> requiredFlags) const
	{
		bool result;
		OperationArgument argument = {*this};
		argument.m_typeArgs[0] = &requiredFlags;
		argument.m_typeArgs[1] = &result;
		m_manager(Operation::GetHasDynamicCompressedData, argument);
		return result;
	}

	uint32 TypeDefinition::CalculateFixedCompressedDataSize(EnumFlags<Reflection::PropertyFlags> requiredFlags) const
	{
		uint32 result;
		OperationArgument argument = {*this};
		argument.m_typeArgs[0] = &requiredFlags;
		argument.m_typeArgs[1] = &result;
		m_manager(Operation::CalculateFixedCompressedDataSize, argument);
		return result;
	}

	uint32 TypeDefinition::CalculateObjectDynamicCompressedDataSize(
		const ConstStoredType source, EnumFlags<Reflection::PropertyFlags> requiredFlags
	) const
	{
		uint32 result;
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = const_cast<StoredType>(source);
		argument.m_storedObjectArgs.m_args[0] = &requiredFlags;
		argument.m_storedObjectArgs.m_args[1] = &result;
		m_manager(Operation::CalculateObjectDynamicCompressedDataSize, argument);
		return result;
	}

	bool TypeDefinition::CompressStoredObject(
		const ConstStoredType source, BitView& targetView, EnumFlags<Reflection::PropertyFlags> requiredFlags
	) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = const_cast<StoredType>(source);
		argument.m_storedObjectArgs.m_args[0] = &targetView;
		argument.m_storedObjectArgs.m_args[1] = &requiredFlags;
		m_manager(Operation::CompressStoredObject, argument);
		return argument.m_storedObjectArgs.pObject != nullptr;
	}

	bool TypeDefinition::DecompressStoredObject(
		const StoredType target, ConstBitView& sourceView, EnumFlags<Reflection::PropertyFlags> requiredFlags
	) const
	{
		OperationArgument argument = {*this};
		argument.m_storedObjectArgs.pObject = target;
		argument.m_storedObjectArgs.m_args[0] = &sourceView;
		argument.m_storedObjectArgs.m_args[1] = &requiredFlags;
		m_manager(Operation::DecompressStoredObject, argument);
		return argument.m_storedObjectArgs.pObject != nullptr;
	}

	namespace Internal
	{
		void GenericTypeDefinition<void>::Manage(const Operation operation, OperationArgument& argument)
		{
			switch (operation)
			{
				case Operation::GetTypeSize:
					*reinterpret_cast<uint32*>(argument.m_typeArgs[0]) = 0;
					break;
				case Operation::GetHasDynamicCompressedData:
					*reinterpret_cast<bool*>(argument.m_typeArgs[1]) = false;
					break;
				case Operation::CalculateFixedCompressedDataSize:
				case Operation::CalculateObjectDynamicCompressedDataSize:
					*reinterpret_cast<uint32*>(argument.m_typeArgs[1]) = 0;
					break;
				case Operation::GetTypeAlignment:
					*reinterpret_cast<uint16*>(argument.m_typeArgs[0]) = 1;
					break;
				case Operation::GetTypeName:
					*reinterpret_cast<ConstUnicodeStringView*>(argument.m_typeArgs[0]) = MAKE_UNICODE_LITERAL("");
					break;
				case Operation::GetPointerTypeDefinition:
					argument.m_typeArgs[0] = reinterpret_cast<void*>(&GenericTypeDefinition<void*>::Manage);
					break;
				case Operation::GetValueTypeDefinition:
					argument.m_typeArgs[0] = reinterpret_cast<void*>(&GenericTypeDefinition<void>::Manage);
					break;
				case Operation::IsTriviallyCopyable:
					*reinterpret_cast<bool*>(argument.m_typeArgs[0]) = false;
					break;

				case Operation::AreReferencesEqual:
					argument.m_objectReferenceArgs.m_args[0] = reinterpret_cast<void*>(true);
					break;
				case Operation::GetReferenceByteView:
					*static_cast<ByteView*>(argument.m_objectReferenceArgs.m_args[0]) = {};
					break;
				case Operation::ReferencePlacementNewDefaultConstruct:
					break;
				case Operation::ReferencePlacementNewCopyConstruct:
					break;
				case Operation::ReferencePlacementNewMoveConstruct:
					break;

				case Operation::GetStoredObjectReference:
				{
					argument.m_storedObjectArgs.pObject = nullptr;
				}
				break;
				case Operation::StoredObjectDestroy:
				{
				}
				break;
				case Operation::AreStoredObjectsEqual:
					argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(true);
					break;
				case Operation::StoredObjectPlacementNewDefaultConstruct:
					argument.m_storedObjectArgs.m_args[0] = nullptr;
					break;
				case Operation::StoredObjectPlacementNewCopyConstruct:
					argument.m_storedObjectArgs.m_args[0] = nullptr;
					break;
				case Operation::StoredObjectPlacementNewMoveConstruct:
					argument.m_storedObjectArgs.m_args[0] = nullptr;
					break;
				case Operation::SerializeStoredObject:
				case Operation::DeserializeStoredObject:
				case Operation::DeserializeConstructStoredObject:
					argument.m_storedObjectArgs.pObject = nullptr;
					break;
				case Operation::CompressStoredObject:
				case Operation::DecompressStoredObject:
					argument.m_storedObjectArgs.pObject = nullptr;
					break;
			}
		}
	}
}

#include <Common/Reflection/DynamicTypeDefinition.h>
#include <Common/Reflection/DynamicPropertyInfo.h>

#include <Common/Serialization/Guid.h>
#include <Common/Memory/Containers/Serialization/Vector.h>
#include <Common/Memory/Align.h>
#include <Common/Memory/CountBits.h>

#include <Common/Reflection/Registry.h>
#include <Common/Reflection/TypeInitializer.h>
#include <Common/Reflection/TypeDeserializer.h>
#include <Common/Reflection/GenericType.h>
#include <Common/Reflection/Serialization/Type.h>
#include <Common/System/Query.h>

namespace ngine::Reflection
{
	/* static */ void DynamicTypeDefinition::Manage(const Operation operation, OperationArgument& argument)
	{
		const DynamicTypeDefinition& typeDefinition = *reinterpret_cast<const DynamicTypeDefinition*>(argument.m_type.GetUserData());

		switch (operation)
		{
			case Operation::GetTypeSize:
				*reinterpret_cast<uint32*>(argument.m_typeArgs[0]) = typeDefinition.m_size;
				break;
			case Operation::GetHasDynamicCompressedData:
			{
				const EnumFlags<Reflection::PropertyFlags> requiredFlags =
					*reinterpret_cast<EnumFlags<Reflection::PropertyFlags>*>(argument.m_typeArgs[0]);
				*reinterpret_cast<bool*>(argument.m_typeArgs[1]) = typeDefinition.HasDynamicCompressedData(requiredFlags);
			}
			break;
			case Operation::CalculateFixedCompressedDataSize:
			{
				const EnumFlags<Reflection::PropertyFlags> requiredFlags =
					*reinterpret_cast<EnumFlags<Reflection::PropertyFlags>*>(argument.m_typeArgs[0]);
				*reinterpret_cast<uint32*>(argument.m_typeArgs[1]) = typeDefinition.CalculateFixedCompressedDataSize(requiredFlags);
			}
			break;
			case Operation::GetTypeAlignment:
				*reinterpret_cast<uint16*>(argument.m_typeArgs[0]) = typeDefinition.m_alignment;
				break;
			case Operation::GetTypeName:
				*reinterpret_cast<ConstUnicodeStringView*>(argument.m_typeArgs[0]) = typeDefinition.m_name;
				break;
			case Operation::GetPointerTypeDefinition:
			case Operation::GetValueTypeDefinition:
				argument.m_typeArgs[0] = reinterpret_cast<void*>(&DynamicTypeDefinition::Manage);
				break;
			case Operation::IsTriviallyCopyable:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : typeDefinition.m_properties)
				{
					if (!propertyInfo.m_typeDefinition.IsTriviallyCopyable())
					{
						*reinterpret_cast<bool*>(argument.m_typeArgs[0]) = false;
						return;
					}
				}
				*reinterpret_cast<bool*>(argument.m_typeArgs[0]) = true;
			}
			break;

			case Operation::AreReferencesEqual:
			{
				argument.m_objectReferenceArgs.m_args[0] = reinterpret_cast<void*>(
					typeDefinition.AreEqual(argument.m_objectReferenceArgs.pObject, argument.m_objectReferenceArgs.m_args[0])
				);
			}
			break;
			case Operation::GetReferenceByteView:
			{
				*static_cast<ByteView*>(argument.m_objectReferenceArgs.m_args[0]
				) = ByteView(static_cast<ByteType*>(argument.m_objectReferenceArgs.pObject), typeDefinition.m_size);
			}
			break;
			case Operation::ReferencePlacementNewDefaultConstruct:
			{
				typeDefinition.ConstructAt(argument.m_objectReferenceArgs.pObject);
			}
			break;
			case Operation::ReferencePlacementNewCopyConstruct:
			{
				typeDefinition.CopyConstructAt(argument.m_objectReferenceArgs.pObject, argument.m_objectReferenceArgs.m_args[0]);
			}
			break;
			case Operation::ReferencePlacementNewMoveConstruct:
			{
				typeDefinition.MoveConstructAt(argument.m_objectReferenceArgs.pObject, argument.m_objectReferenceArgs.m_args[0]);
			}
			break;

			case Operation::GetStoredObjectReference:
				break;
			case Operation::StoredObjectDestroy:
			{
				typeDefinition.DestroyAt(argument.m_storedObjectArgs.pObject);
			}
			break;
			case Operation::AreStoredObjectsEqual:
			{
				argument.m_objectReferenceArgs.pObject = reinterpret_cast<void*>(
					typeDefinition.AreEqual(argument.m_objectReferenceArgs.pObject, argument.m_objectReferenceArgs.m_args[0])
				);
			}
			break;
			case Operation::StoredObjectPlacementNewDefaultConstruct:
			{
				uint32& availableMemorySize = *reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[0]);

				const uint32 alignmentOffset =
					static_cast<uint32>(reinterpret_cast<uintptr>(argument.m_storedObjectArgs.pObject) % typeDefinition.m_alignment);
				if (typeDefinition.m_size + alignmentOffset <= availableMemorySize)
				{
					typeDefinition.ConstructAt(argument.m_objectReferenceArgs.pObject);
					availableMemorySize = 0;
				}
				else
				{
					// Return uint64 with both type size and required alignment
					const uint32 maximumRequiredSize = typeDefinition.m_size + typeDefinition.m_alignment;
					availableMemorySize = maximumRequiredSize;
				}
			}
			break;
			case Operation::StoredObjectPlacementNewCopyConstruct:
			{
				uint32& availableMemorySize = *reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[0]);

				const uint32 alignmentOffset =
					static_cast<uint32>(reinterpret_cast<uintptr>(argument.m_storedObjectArgs.pObject) % typeDefinition.m_alignment);
				if (typeDefinition.m_size + alignmentOffset <= availableMemorySize)
				{
					typeDefinition.CopyConstructAt(argument.m_objectReferenceArgs.pObject, argument.m_objectReferenceArgs.m_args[1]);
					availableMemorySize = 0;
				}
				else
				{
					const uint32 maximumRequiredSize = typeDefinition.m_size + typeDefinition.m_alignment;
					availableMemorySize = maximumRequiredSize;
				}
			}
			break;
			case Operation::StoredObjectPlacementNewMoveConstruct:
			{
				uint32& availableMemorySize = *reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[0]);

				const uint32 alignmentOffset =
					static_cast<uint32>(reinterpret_cast<uintptr>(argument.m_storedObjectArgs.pObject) % typeDefinition.m_alignment);
				if (typeDefinition.m_size + alignmentOffset <= availableMemorySize)
				{
					typeDefinition.MoveConstructAt(argument.m_objectReferenceArgs.pObject, argument.m_objectReferenceArgs.m_args[1]);
					availableMemorySize = 0;
				}
				else
				{
					const uint32 maximumRequiredSize = typeDefinition.m_size + typeDefinition.m_alignment;
					availableMemorySize = maximumRequiredSize;
				}
			}
			break;
			case Operation::SerializeStoredObject:
			{
				Serialization::Writer& serializer = *reinterpret_cast<Serialization::Writer*>(argument.m_storedObjectArgs.m_args[0]);
				const bool success = typeDefinition.Serialize(argument.m_objectReferenceArgs.pObject, serializer);
				argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
			}
			break;
			case Operation::DeserializeStoredObject:
			{
				const Serialization::Reader& serializer = *reinterpret_cast<const Serialization::Reader*>(argument.m_storedObjectArgs.m_args[0]);
				const bool success = typeDefinition.Serialize(argument.m_objectReferenceArgs.pObject, serializer);
				argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
			}
			break;
			case Operation::DeserializeConstructStoredObject:
			{
				const Serialization::Reader& serializer = *reinterpret_cast<const Serialization::Reader*>(argument.m_storedObjectArgs.m_args[0]);
				typeDefinition.ConstructAt(argument.m_objectReferenceArgs.pObject);
				const bool success = typeDefinition.Serialize(argument.m_objectReferenceArgs.pObject, serializer);
				argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
			}
			break;
			case Operation::CalculateObjectDynamicCompressedDataSize:
			{
				const EnumFlags<Reflection::PropertyFlags> requiredFlags =
					*reinterpret_cast<EnumFlags<Reflection::PropertyFlags>*>(argument.m_storedObjectArgs.m_args[0]);
				*reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[1]
				) = typeDefinition.CalculateDynamicCompressedDataSize(argument.m_storedObjectArgs.pObject, requiredFlags);
			}
			break;
			case Operation::CompressStoredObject:
			{
				BitView& targetView = *reinterpret_cast<BitView*>(argument.m_storedObjectArgs.m_args[0]);
				const EnumFlags<Reflection::PropertyFlags> requiredFlags =
					*reinterpret_cast<EnumFlags<Reflection::PropertyFlags>*>(argument.m_storedObjectArgs.m_args[1]);
				const bool success = typeDefinition.Compress(argument.m_storedObjectArgs.pObject, targetView, requiredFlags);
				argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
			}
			break;
			case Operation::DecompressStoredObject:
			{
				ConstBitView& sourceView = *reinterpret_cast<ConstBitView*>(argument.m_storedObjectArgs.m_args[0]);
				const EnumFlags<Reflection::PropertyFlags> requiredFlags =
					*reinterpret_cast<EnumFlags<Reflection::PropertyFlags>*>(argument.m_storedObjectArgs.m_args[1]);
				const bool success = typeDefinition.Decompress(argument.m_storedObjectArgs.pObject, sourceView, requiredFlags);
				argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
			}
			break;
		}
	}

	/* static */ void DynamicTypeDefinition::IterateProperties(const TypeInterface& typeInterface, IteratePropertiesCallback&& callback)
	{
		const DynamicTypeDefinition& typeDefinition = static_cast<const DynamicTypeDefinition&>(typeInterface);

		switch (typeDefinition.m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : typeDefinition.m_properties)
				{
					callback(
						propertyInfo,
						DynamicPropertyInstance(
							[offset = propertyInfo.m_offsetFromOwner,
					     typeDefinition =
					       propertyInfo.m_typeDefinition](PropertyOwner& owner, [[maybe_unused]] const Optional<PropertyOwner*> pParent)
							{
								uintptr address = reinterpret_cast<uintptr>(&owner) + offset;
								return Any(reinterpret_cast<void*>(address), typeDefinition);
							},
							[offset = propertyInfo.m_offsetFromOwner, typeDefinition = propertyInfo.m_typeDefinition](
								PropertyOwner& owner,
								[[maybe_unused]] const Optional<PropertyOwner*> pParent,
								Any&& newValue
							)
							{
								uintptr address = reinterpret_cast<uintptr>(&owner) + offset;
								const AnyView valueView(reinterpret_cast<void*>(address), typeDefinition);
								newValue.MoveIntoView(valueView);
							}
						)
					);
				}
			}
			break;
			case Type::Variant:
			{
				// Variant types don't count as properties
			}
			break;
		}
	}

	/* static */ void DynamicTypeDefinition::IterateFunctions(const TypeInterface& typeInterface, IterateFunctionsCallback&&)
	{
		[[maybe_unused]] const DynamicTypeDefinition& typeDefinition = static_cast<const DynamicTypeDefinition&>(typeInterface);
	}

	/* static */ void DynamicTypeDefinition::IterateEvents(const TypeInterface& typeInterface, IterateEventsCallback&&)
	{
		[[maybe_unused]] const DynamicTypeDefinition& typeDefinition = static_cast<const DynamicTypeDefinition&>(typeInterface);
	}

	DynamicTypeDefinition::DynamicTypeDefinition()
		: TypeDefinition(&DynamicTypeDefinition::Manage, this)
		, TypeInterface(Guid(), {}, {}, {}, *this, nullptr)
	{
		m_iteratePropertiesFunction = IterateProperties;
		m_iterateFunctionsFunction = IterateFunctions;
		m_iterateEventsFunction = IterateEvents;
	}

	DynamicTypeDefinition::DynamicTypeDefinition(
		const Guid typeGuid,
		const ConstUnicodeStringView name,
		const ConstUnicodeStringView description,
		const EnumFlags<TypeFlags> flags,
		const TypeInterface* pParent
	)
		: TypeDefinition(&DynamicTypeDefinition::Manage, this)
		, TypeInterface(typeGuid, name, description, flags, *this, pParent)
		, m_storedName(name)
	{
		m_name = m_storedName;
		m_iteratePropertiesFunction = IterateProperties;
		m_iterateFunctionsFunction = IterateFunctions;
		m_iterateEventsFunction = IterateEvents;
	}

	void DynamicTypeDefinition::SetName(UnicodeString&& name)
	{
		m_storedName = name;
		m_name = m_storedName;
	}

	/* static */ DynamicTypeDefinition DynamicTypeDefinition::MakeStructure(Vector<DynamicPropertyInfo>&& properties)
	{
		DynamicTypeDefinition typeDefinition;
		typeDefinition.m_type = Type::Structure;
		typeDefinition.m_properties = Forward<Vector<DynamicPropertyInfo>>(properties);
		typeDefinition.RecalculateMemoryData();
		return typeDefinition;
	}

	/* static */ DynamicTypeDefinition DynamicTypeDefinition::MakeStructure(const ArrayView<const Reflection::TypeDefinition> types)
	{
		DynamicTypeDefinition typeDefinition;
		typeDefinition.m_type = Type::Structure;
		typeDefinition.m_properties.Reserve(types.GetSize());
		for (const Reflection::TypeDefinition type : types)
		{
			typeDefinition.m_properties.EmplaceBack(Reflection::PropertyInfo{
				MAKE_UNICODE_LITERAL("Dynamic"),
				"dynamic1",
				Guid::Generate(),
				typeDefinition.m_name,
				Reflection::PropertyFlags::Transient | Reflection::PropertyFlags::HideFromUI,
				Guid{},
				type
			});
		}
		typeDefinition.RecalculateMemoryData();
		return typeDefinition;
	}

	/* static */ DynamicTypeDefinition DynamicTypeDefinition::MakeVariant(const ArrayView<const Reflection::TypeDefinition> types)
	{
		DynamicTypeDefinition typeDefinition;
		typeDefinition.m_type = Type::Variant;
		typeDefinition.m_properties.Reserve(types.GetSize());
		for (const Reflection::TypeDefinition type : types)
		{
			typeDefinition.m_properties.EmplaceBack(
				Reflection::PropertyInfo{MAKE_UNICODE_LITERAL(""), "", Guid{}, typeDefinition.m_name, Reflection::PropertyFlags{}, Guid{}, type}
			);
		}
		typeDefinition.RecalculateMemoryData();
		return typeDefinition;
	}

	void DynamicTypeDefinition::RecalculateMemoryData()
	{
		m_alignment = 1;
		for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
		{
			m_alignment = Math::Max(m_alignment, propertyInfo.m_typeDefinition.GetAlignment());
		}

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				DynamicPropertyInfo dummyProperty;
				ReferenceWrapper<DynamicPropertyInfo> previousProperty = dummyProperty;

				uint32 address = 0;
				address += address % m_alignment;
				for (DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					propertyInfo.m_offsetFromOwner = address;
					previousProperty->m_offsetToNextProperty = propertyInfo.m_offsetFromOwner - previousProperty->m_offsetFromOwner;

					previousProperty = propertyInfo;
					address += propertyInfo.m_typeDefinition.GetSize();
				}

				if (m_properties.HasElements())
				{
					previousProperty->m_offsetToNextProperty = 0;

					const DynamicPropertyInfo& __restrict lastPropertyInfo = m_properties.GetLastElement();
					m_size = lastPropertyInfo.m_offsetFromOwner + lastPropertyInfo.m_typeDefinition.GetSize();
				}
				else
				{
					m_size = 0;
				}
			}
			break;
			case Type::Variant:
			{
				uint32 typeSize{0};
				for (const DynamicPropertyInfo& property : m_properties)
				{
					typeSize = Math::Max(typeSize, property.m_typeDefinition.GetSize());
				}
				m_size = Memory::Align((uint32)sizeof(VariantIndexType), m_alignment) + typeSize;
			}
			break;
		}
	}

	void DynamicTypeDefinition::ConstructAt(void* pAddress) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.DefaultConstructAt(reinterpret_cast<void*>(address));

					address += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				VariantIndexType& activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				activeIndex = 0;
			}
			break;
		}
	}

	void DynamicTypeDefinition::CopyConstructAt(void* pAddress, const void* pSourceAddress) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));
		Assert(Memory::IsAligned(pSourceAddress, m_alignment));
		uintptr address = reinterpret_cast<uintptr>(pAddress);
		uintptr sourceAddress = reinterpret_cast<uintptr>(pSourceAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));
					Assert(Memory::IsAligned(sourceAddress, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.CopyConstructAt(reinterpret_cast<void*>(address), reinterpret_cast<const void*>(sourceAddress));

					address += propertyInfo.m_offsetToNextProperty;
					sourceAddress += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				VariantIndexType& activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				const VariantIndexType sourceActiveIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(sourceAddress));
				address += Memory::Align(sizeof(VariantIndexType), m_alignment);
				sourceAddress += Memory::Align(sizeof(VariantIndexType), m_alignment);

				activeIndex = sourceActiveIndex;
				if (sourceActiveIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[sourceActiveIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));
					Assert(Memory::IsAligned(sourceAddress, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.CopyConstructAt(reinterpret_cast<void*>(address), reinterpret_cast<const void*>(sourceAddress));
				}
			}
			break;
		}
	}

	void DynamicTypeDefinition::MoveConstructAt(void* pAddress, void* pSourceAddress) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));
		Assert(Memory::IsAligned(pSourceAddress, m_alignment));
		uintptr address = reinterpret_cast<uintptr>(pAddress);
		uintptr sourceAddress = reinterpret_cast<uintptr>(pSourceAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));
					Assert(Memory::IsAligned(sourceAddress, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.MoveConstructAt(reinterpret_cast<void*>(address), reinterpret_cast<void*>(sourceAddress));

					address += propertyInfo.m_offsetToNextProperty;
					sourceAddress += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				VariantIndexType& activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				const VariantIndexType sourceActiveIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(sourceAddress));
				address += Memory::Align(sizeof(VariantIndexType), m_alignment);
				sourceAddress += Memory::Align(sizeof(VariantIndexType), m_alignment);

				if (sourceActiveIndex != 0)
				{
					activeIndex = sourceActiveIndex;

					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[sourceActiveIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));
					Assert(Memory::IsAligned(sourceAddress, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.MoveConstructAt(reinterpret_cast<void*>(address), reinterpret_cast<void*>(sourceAddress));
				}
			}
			break;
		}
	}

	void DynamicTypeDefinition::DestroyAt(void* pAddress) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.DestroyUnalignedObject(reinterpret_cast<void*>(address));

					address += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				VariantIndexType& activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				address += Memory::Align(sizeof(VariantIndexType), m_alignment);

				if (activeIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[activeIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					propertyInfo.m_typeDefinition.DestroyUnalignedObject(reinterpret_cast<void*>(address));
				}
			}
			break;
		}
	}

	bool DynamicTypeDefinition::AreEqual(const void* pLeftAddress, const void* pRightAddress) const
	{
		Assert(Memory::IsAligned(pLeftAddress, m_alignment));
		Assert(Memory::IsAligned(pRightAddress, m_alignment));

		uintptr leftAddress = reinterpret_cast<uintptr>(pLeftAddress);
		uintptr rightAddress = reinterpret_cast<uintptr>(pRightAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(leftAddress, propertyInfo.m_typeDefinition.GetAlignment()));
					Assert(Memory::IsAligned(rightAddress, propertyInfo.m_typeDefinition.GetAlignment()));

					if (!propertyInfo.m_typeDefinition
					       .AreStoredObjectsEqual(reinterpret_cast<void*>(leftAddress), reinterpret_cast<void*>(rightAddress)))
					{
						return false;
					}

					leftAddress += propertyInfo.m_offsetToNextProperty;
					rightAddress += propertyInfo.m_offsetToNextProperty;
				}

				return true;
			}
			case Type::Variant:
			{
				const VariantIndexType leftActiveIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(leftAddress));
				const VariantIndexType rightActiveIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(rightAddress));
				leftAddress += Memory::Align(sizeof(VariantIndexType), m_alignment);
				rightAddress += Memory::Align(sizeof(VariantIndexType), m_alignment);

				if (leftActiveIndex != rightActiveIndex)
				{
					return false;
				}

				if (leftActiveIndex == 0)
				{
					return true;
				}

				const DynamicPropertyInfo& __restrict propertyInfo = m_properties[leftActiveIndex - 1];

				Assert(Memory::IsAligned(leftAddress, propertyInfo.m_typeDefinition.GetAlignment()));
				Assert(Memory::IsAligned(rightAddress, propertyInfo.m_typeDefinition.GetAlignment()));

				return propertyInfo.m_typeDefinition
				  .AreStoredObjectsEqual(reinterpret_cast<void*>(leftAddress), reinterpret_cast<void*>(rightAddress));
			}
		}
		ExpectUnreachable();
	}

	DynamicTypeDefinition::PropertyQueryResult DynamicTypeDefinition::FindProperty(const Guid propertyGuid) const
	{
		for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
		{
			if (propertyInfo.m_guid == propertyGuid)
			{
				return PropertyQueryResult{
					&propertyInfo,
					DynamicPropertyInstance(
						[offset = propertyInfo.m_offsetFromOwner,
				     typeDefinition = propertyInfo.m_typeDefinition](PropertyOwner& owner, [[maybe_unused]] const Optional<PropertyOwner*> pParent)
						{
							uintptr address = reinterpret_cast<uintptr>(&owner) + offset;
							return Any(reinterpret_cast<void*>(address), typeDefinition);
						},
						[offset = propertyInfo.m_offsetFromOwner,
				     typeDefinition =
				       propertyInfo.m_typeDefinition](PropertyOwner& owner, [[maybe_unused]] const Optional<PropertyOwner*> pParent, Any&& newValue)
						{
							uintptr address = reinterpret_cast<uintptr>(&owner) + offset;
							const AnyView valueView(reinterpret_cast<void*>(address), typeDefinition);
							newValue.MoveIntoView(valueView);
						}
					)
				};
			}
		}
		return {};
	}

	Optional<const FunctionInfo*> DynamicTypeDefinition::FindFunction([[maybe_unused]] const Guid functionGuid) const
	{
		Assert(false, "Functions not supported in dynamic types!");
		return Invalid;
	}

	Optional<const EventInfo*> DynamicTypeDefinition::FindEvent([[maybe_unused]] const Guid eventGuid) const
	{
		Assert(false, "Events not supported in dynamic types!");
		return Invalid;
	}

	ArrayView<const Tag::Guid> DynamicTypeDefinition::GetTags() const
	{
		// TODO: Implement support for tags on dynamic assets
		return {};
	}

	Optional<const ExtensionInterface*> DynamicTypeDefinition::FindExtension([[maybe_unused]] const Guid typeGuid) const
	{
		// TODO: Implement support for extensions on dynamic assets
		return {};
	}

	bool DynamicPropertyInfo::Serialize(const Serialization::Reader reader)
	{
		reader.Serialize("display_name", m_storedDisplayName);
		m_displayName = m_storedDisplayName;
		reader.Serialize("name", m_storedName);
		m_name = m_storedName;
		reader.Serialize("category_display_name", m_storedCategoryDisplayName);
		m_categoryDisplayName = m_storedCategoryDisplayName;
		reader.Serialize("guid", m_guid);

		Reflection::Registry& registry = System::Get<Reflection::Registry>();
		if (const Optional<const TypeDefinition*> typeDefinition = registry.FindTypeDefinition(*reader.Read<Guid>("type")))
		{
			m_typeDefinition = *typeDefinition;

			if (reader.ReadWithDefaultValue<bool>("pointer", false))
			{
				m_typeDefinition = m_typeDefinition.GetPointerTypeDefinition();
			}
		}
		else
		{
			return false;
		}

		reader.Serialize("owner_offset", m_offsetFromOwner);
		reader.Serialize("offset_to_next", m_offsetToNextProperty);
		return true;
	}

	bool DynamicPropertyInfo::Serialize(Serialization::Writer writer) const
	{
		writer.Serialize("display_name", m_storedDisplayName);
		writer.Serialize("name", m_storedName);
		writer.Serialize("category_display_name", m_storedCategoryDisplayName);
		writer.Serialize("guid", m_guid);

		const TypeDefinition valueTypeDefinition = m_typeDefinition.GetValueTypeDefinition();
		Reflection::Registry& registry = System::Get<Reflection::Registry>();
		writer.Serialize("type", registry.FindTypeDefinitionGuid(valueTypeDefinition));
		if (valueTypeDefinition != m_typeDefinition)
		{
			writer.Serialize("pointer", true);
		}

		writer.Serialize("owner_offset", m_offsetFromOwner);
		writer.Serialize("offset_to_next", m_offsetToNextProperty);
		return true;
	}

	bool DynamicTypeDefinition::Serialize(const Serialization::Reader reader)
	{
		reader.Serialize("type", m_type);
		reader.Serialize("guid", m_guid);
		reader.Serialize("name", m_storedName);
		m_name = m_storedName;
		// TODO: Allow parsing and handling parent types

		if (!reader.Serialize("properties", m_properties))
		{
			return false;
		}

		RecalculateMemoryData();

		return true;
	}

	bool DynamicTypeDefinition::Serialize(Serialization::Writer writer) const
	{
		writer.Serialize("type", m_type);
		writer.Serialize("guid", m_guid);
		writer.Serialize("name", m_storedName);
		// TODO: Allow parsing and handling parent types
		writer.Serialize("properties", m_properties);
		return true;
	}

	bool DynamicTypeDefinition::Serialize(void* pAddress, const Serialization::Reader reader) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));

		bool failedAny = false;
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					failedAny |= !propertyInfo.m_typeDefinition.SerializeStoredObject(reinterpret_cast<void*>(address), reader);

					address += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				VariantIndexType& activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				reader.Serialize("active", activeIndex);

				address += Memory::Align(sizeof(VariantIndexType), m_alignment);

				if (activeIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[activeIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					const Optional<Serialization::Reader> objectReader = reader.FindSerializer("object");
					Assert(objectReader.IsValid());
					if (LIKELY(objectReader.IsValid()))
					{
						failedAny |= !propertyInfo.m_typeDefinition.SerializeStoredObject(reinterpret_cast<void*>(address), *objectReader);
					}
				}
			}
			break;
		}

		return !failedAny;
	}

	bool DynamicTypeDefinition::Serialize(const void* pAddress, Serialization::Writer writer) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));

		bool failedAny = false;
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					failedAny |= !propertyInfo.m_typeDefinition.SerializeStoredObject(reinterpret_cast<const void*>(address), writer);

					address += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				const VariantIndexType activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				writer.Serialize("active", activeIndex);

				address += Memory::Align(sizeof(VariantIndexType), m_alignment);

				if (activeIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[activeIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					Serialization::Writer objectWriter = writer.EmplaceObjectElement("object");
					failedAny |= !propertyInfo.m_typeDefinition.SerializeStoredObject(reinterpret_cast<const void*>(address), objectWriter);
				}
			}
			break;
		}

		return !failedAny;
	}

	uint32 DynamicTypeDefinition::CalculateFixedCompressedDataSize(EnumFlags<Reflection::PropertyFlags> requiredFlags) const
	{
		uint32 compressedSize{0};

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					compressedSize += propertyInfo.m_typeDefinition.CalculateFixedCompressedDataSize(requiredFlags);
				}
			}
			break;
			case Type::Variant:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					compressedSize = Math::Max(compressedSize, propertyInfo.m_typeDefinition.CalculateFixedCompressedDataSize(requiredFlags));
				}
			}
			break;
		}
		return compressedSize;
	}

	uint32
	DynamicTypeDefinition::CalculateDynamicCompressedDataSize(const void* pAddress, EnumFlags<Reflection::PropertyFlags> requiredFlags) const
	{
		uint32 compressedSize{0};
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					compressedSize +=
						propertyInfo.m_typeDefinition.CalculateObjectDynamicCompressedDataSize(reinterpret_cast<void*>(address), requiredFlags);
				}
			}
			break;
			case Type::Variant:
			{
				const VariantIndexType activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));

				compressedSize += Memory::GetBitWidth(m_properties.GetSize());

				if (activeIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[activeIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					compressedSize +=
						propertyInfo.m_typeDefinition.CalculateObjectDynamicCompressedDataSize(reinterpret_cast<void*>(address), requiredFlags);
				}
			}
			break;
		}
		return compressedSize;
	}

	bool
	DynamicTypeDefinition::Compress(const void* pAddress, BitView& target, const EnumFlags<Reflection::PropertyFlags> requiredFlags) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));

		bool failedAny = false;
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					failedAny |= !propertyInfo.m_typeDefinition.CompressStoredObject(reinterpret_cast<const void*>(address), target, requiredFlags);

					address += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				const VariantIndexType activeIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));

				const uint32 indexBitCount = Memory::GetBitWidth(m_properties.GetSize());
				target.PackAndSkip(ConstBitView::Make(activeIndex, Math::Range<size>::Make(0, indexBitCount)));

				if (activeIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[activeIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					failedAny |= !propertyInfo.m_typeDefinition.CompressStoredObject(reinterpret_cast<const void*>(address), target, requiredFlags);
				}
			}
			break;
		}

		return !failedAny;
	}

	bool
	DynamicTypeDefinition::Decompress(void* pAddress, ConstBitView& source, const EnumFlags<Reflection::PropertyFlags> requiredFlags) const
	{
		Assert(Memory::IsAligned(pAddress, m_alignment));

		bool failedAny = false;
		uintptr address = reinterpret_cast<uintptr>(pAddress);

		switch (m_type)
		{
			case Type::Native:
			case Type::Invalid:
				ExpectUnreachable();
			case Type::Structure:
			{
				for (const DynamicPropertyInfo& __restrict propertyInfo : m_properties)
				{
					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					failedAny |= !propertyInfo.m_typeDefinition.DecompressStoredObject(reinterpret_cast<void*>(address), source, requiredFlags);

					address += propertyInfo.m_offsetToNextProperty;
				}
			}
			break;
			case Type::Variant:
			{
				VariantIndexType& targetActiveIndex = *reinterpret_cast<VariantIndexType*>(reinterpret_cast<void*>(address));
				const VariantIndexType previousActiveIndex = targetActiveIndex;

				const uint32 indexBitCount = Memory::GetBitWidth(m_properties.GetSize());
				const VariantIndexType activeIndex = source.UnpackAndSkip<VariantIndexType>(Math::Range<size>::Make(0, indexBitCount));

				address += Memory::Align(sizeof(VariantIndexType), m_alignment);

				if (previousActiveIndex != activeIndex && previousActiveIndex != 0)
				{
					const DynamicPropertyInfo& propertyInfo = m_properties[previousActiveIndex - 1];
					propertyInfo.m_typeDefinition.DestroyUnalignedObject(reinterpret_cast<void*>(address));
				}

				targetActiveIndex = activeIndex;

				if (activeIndex != 0)
				{
					const DynamicPropertyInfo& __restrict propertyInfo = m_properties[activeIndex - 1];

					Assert(Memory::IsAligned(address, propertyInfo.m_typeDefinition.GetAlignment()));

					failedAny |= !propertyInfo.m_typeDefinition.DecompressStoredObject(reinterpret_cast<void*>(address), source, requiredFlags);
				}
			}
			break;
		}

		return !failedAny;
	}
}

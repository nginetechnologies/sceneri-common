#pragma once

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/TypeTraits/Select.h>
#include <Common/TypeTraits/IsEqualityComparable.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include <Common/TypeTraits/IsTriviallyCopyable.h>
#include <Common/TypeTraits/IsPointer.h>
#include <Common/TypeTraits/IsReference.h>
#include <Common/TypeTraits/TypeName.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/AddressOf.h>
#include "TypeDefinition.h"

namespace ngine::Reflection::Internal
{
	template<typename Type>
	void GenericTypeDefinition<Type>::Manage(const Operation operation, OperationArgument& argument)
	{
		switch (operation)
		{
			case Operation::GetTypeSize:
			{
				static constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<Type>, Type>;
				using FilteredType = TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<Type>>, Type>;
				*reinterpret_cast<uint32*>(argument.m_typeArgs[0]) = sizeof(FilteredType);
			}
			break;
			case Operation::GetTypeAlignment:
			{
				static constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<Type>, Type>;
				using FilteredType = TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<Type>>, Type>;
				*reinterpret_cast<uint16*>(argument.m_typeArgs[0]) = alignof(FilteredType);
			}
			break;
			case Operation::GetTypeName:
			{
				const ConstStringView typeName = TypeTraits::GetTypeName<Type>();
				*reinterpret_cast<ConstUnicodeStringView*>(argument.m_typeArgs[0]) = reinterpret_cast<const ConstUnicodeStringView&>(typeName);
			}
			break;
			case Operation::GetPointerTypeDefinition:
			{
				argument.m_typeArgs[0] = reinterpret_cast<void*>(&GenericTypeDefinition<TypeTraits::WithoutPointer<Type>*>::Manage);
			}
			break;
			case Operation::GetValueTypeDefinition:
			{
				argument.m_typeArgs[0] =
					reinterpret_cast<void*>(&GenericTypeDefinition<TypeTraits::WithoutPointer<TypeTraits::WithoutReference<Type>>>::Manage);
			}
			break;
			case Operation::IsTriviallyCopyable:
			{
				*reinterpret_cast<bool*>(argument.m_typeArgs[0]) = TypeTraits::IsTriviallyCopyable<Type>;
			}
			break;

			case Operation::AreReferencesEqual:
			{
				// TODO: Change this to implement <=> when we switch to C++20
				if constexpr (TypeTraits::IsEqualityComparable<Type, Type>)
				{
					const Type& a = *reinterpret_cast<const Type*>(argument.m_objectReferenceArgs.pObject);
					const Type& b = *reinterpret_cast<const Type*>(argument.m_objectReferenceArgs.m_args[0]);

					argument.m_objectReferenceArgs.m_args[0] = reinterpret_cast<void*>(static_cast<bool>(a == b));
				}
				else
				{
					const Type* a = reinterpret_cast<const Type*>(argument.m_objectReferenceArgs.pObject);
					const Type* b = reinterpret_cast<const Type*>(argument.m_objectReferenceArgs.m_args[0]);

					argument.m_objectReferenceArgs.m_args[0] = reinterpret_cast<void*>(static_cast<bool>(a == b));
					Assert(argument.m_objectReferenceArgs.m_args[0] != nullptr, "Types must be equality comparable!");
				}
			}
			break;
			case Operation::GetReferenceByteView:
			{
				Type& value = *reinterpret_cast<Type*>(argument.m_objectReferenceArgs.pObject);
				if constexpr (TypeTraits::IsConst<Type>)
				{
					*static_cast<ConstByteView*>(argument.m_objectReferenceArgs.m_args[0]) = ConstByteView::Make(value);
				}
				else
				{
					*static_cast<ByteView*>(argument.m_objectReferenceArgs.m_args[0]) = ByteView::Make(value);
				}
			}
			break;
			case Operation::ReferencePlacementNewDefaultConstruct:
			{
				if constexpr (TypeTraits::IsDefaultConstructible<Type>)
				{
					new (argument.m_objectReferenceArgs.pObject) Type();
				}
				else
				{
					Assert(false);
				}
			}
			break;
			case Operation::ReferencePlacementNewCopyConstruct:
			{
				if constexpr (TypeTraits::IsCopyConstructible<Type>)
				{
					const Type& other = *static_cast<const Type*>(argument.m_objectReferenceArgs.m_args[0]);
					new (argument.m_objectReferenceArgs.pObject) Type(other);
				}
				else
				{
					Assert(false);
				}
			}
			break;
			case Operation::ReferencePlacementNewMoveConstruct:
			{
				if constexpr (TypeTraits::IsMoveConstructible<Type>)
				{
					Type& other = *static_cast<Type*>(argument.m_objectReferenceArgs.m_args[0]);
					new (argument.m_objectReferenceArgs.pObject) Type(Move(other));
				}
				else
				{
					Assert(false);
				}
			}
			break;

			case Operation::GetStoredObjectReference:
			{
				Type& object = GetStoredObject(argument.m_storedObjectArgs.pObject);
				argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(Memory::GetAddressOf(object));
			}
			break;
			case Operation::StoredObjectDestroy:
			{
				if constexpr (!TypeTraits::IsTriviallyDestructible<Type>)
				{
					Type& object = GetStoredObject(argument.m_storedObjectArgs.pObject);
					object.~Type();
				}
			}
			break;
			case Operation::AreStoredObjectsEqual:
			{
				// TODO: Change this to implement <=> when we switch to C++20
				if constexpr (TypeTraits::IsEqualityComparable<Type, Type>)
				{
					const Type& a = GetStoredObject(argument.m_storedObjectArgs.pObject);
					const Type& b = GetStoredObject(argument.m_storedObjectArgs.m_args[0]);

					argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(static_cast<bool>(a == b));
				}
				else
				{
					Assert(false);
					argument.m_objectReferenceArgs.pObject = nullptr;
				}
			}
			break;
			case Operation::StoredObjectPlacementNewDefaultConstruct:
			{
				uint32& availableMemorySize = *reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[0]);
				if constexpr (TypeTraits::IsDefaultConstructible<Type>)
				{
					const uint32 alignmentOffset =
						static_cast<uint32>(reinterpret_cast<uintptr>(argument.m_storedObjectArgs.pObject) % alignof(Type));
					if (sizeof(Type) + alignmentOffset <= availableMemorySize)
					{
						Type& value = GetStoredObject(argument.m_storedObjectArgs.pObject);
						new (Memory::GetAddressOf(value)) Type();

						availableMemorySize = 0;
					}
					else
					{
						constexpr uint32 MaximumRequiredSize = sizeof(Type) + alignof(Type);
						availableMemorySize = MaximumRequiredSize;
					}
				}
				else
				{
					Assert(false);
					availableMemorySize = Math::NumericLimits<uint32>::Max;
				}
			}
			break;
			case Operation::StoredObjectPlacementNewCopyConstruct:
			{
				uint32& availableMemorySize = *reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[0]);
				if constexpr (TypeTraits::IsCopyConstructible<Type>)
				{
					const uint32 alignmentOffset =
						static_cast<uint32>(reinterpret_cast<uintptr>(argument.m_storedObjectArgs.pObject) % alignof(Type));
					if (sizeof(Type) + alignmentOffset <= availableMemorySize)
					{
						Type& value = GetStoredObject(argument.m_storedObjectArgs.pObject);
						const Type& copiedObject = GetStoredObject(argument.m_storedObjectArgs.m_args[1]);
						Assert((reinterpret_cast<uintptr>(&copiedObject) % alignof(Type)) == 0);
						new (Memory::GetAddressOf(value)) Type(copiedObject);

						availableMemorySize = 0;
					}
					else
					{
						static constexpr uint32 MaximumRequiredSize = sizeof(Type) + alignof(Type);
						availableMemorySize = MaximumRequiredSize;
					}
				}
				else
				{
					Assert(false);
					availableMemorySize = Math::NumericLimits<uint32>::Max;
				}
			}
			break;
			case Operation::StoredObjectPlacementNewMoveConstruct:
			{
				uint32& availableMemorySize = *reinterpret_cast<uint32*>(argument.m_storedObjectArgs.m_args[0]);
				if constexpr (TypeTraits::IsMoveConstructible<Type>)
				{
					const uint32 alignmentOffset =
						static_cast<uint32>(reinterpret_cast<uintptr>(argument.m_storedObjectArgs.pObject) % alignof(Type));
					if (sizeof(Type) + alignmentOffset <= availableMemorySize)
					{
						Type& value = GetStoredObject(argument.m_storedObjectArgs.pObject);
						Type& movedObject = GetStoredObject(argument.m_storedObjectArgs.m_args[1]);
						Assert((reinterpret_cast<uintptr>(&movedObject) % alignof(Type)) == 0);
						new (Memory::GetAddressOf(value)) Type(Move(movedObject));

						availableMemorySize = 0;
					}
					else
					{
						static constexpr uint32 MaximumRequiredSize = sizeof(Type) + alignof(Type);
						availableMemorySize = MaximumRequiredSize;
					}
				}
				else
				{
					Assert(false);
					availableMemorySize = Math::NumericLimits<uint32>::Max;
				}
			}
			break;
			case Operation::SerializeStoredObject:
			{
				Serialization::Writer& serializer = *reinterpret_cast<Serialization::Writer*>(argument.m_storedObjectArgs.m_args[0]);
				if constexpr (Serialization::Internal::CanWrite<Type>)
				{
					const Type& value = GetStoredObject(argument.m_storedObjectArgs.pObject);
					const bool success = Serialization::Internal::SerializeElement<Type>(value, serializer);
					argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
				}
				else
				{
					argument.m_storedObjectArgs.pObject = nullptr;
				}
			}
			break;
			case Operation::DeserializeStoredObject:
			{
				const Serialization::Reader& serializer = *reinterpret_cast<const Serialization::Reader*>(argument.m_storedObjectArgs.m_args[0]);
				if constexpr (Serialization::Internal::CanRead<Type>)
				{
					Type& value = GetStoredObject(argument.m_storedObjectArgs.pObject);
					const bool success = Serialization::Internal::DeserializeElement<Type>(value, serializer);
					argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
				}
				else
				{
					argument.m_storedObjectArgs.pObject = nullptr;
				}
			}
			break;
			case Operation::DeserializeConstructStoredObject:
			{
				const Serialization::Reader& serializer = *reinterpret_cast<const Serialization::Reader*>(argument.m_storedObjectArgs.m_args[0]);
				if constexpr (Serialization::Internal::CanRead<Type> && TypeTraits::IsDefaultConstructible<Type>)
				{
					Type& value = GetStoredObject(argument.m_storedObjectArgs.pObject);
					new (Memory::GetAddressOf(value)) Type();
					const bool success = Serialization::Internal::DeserializeElement(value, serializer);
					argument.m_storedObjectArgs.pObject = reinterpret_cast<void*>(success);
				}
				else
				{
					argument.m_storedObjectArgs.pObject = nullptr;
				}
			}
			break;
		}
	}

	template void GenericTypeDefinition<void*>::Manage(const Operation operation, OperationArgument& argument);
}

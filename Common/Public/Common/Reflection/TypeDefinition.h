#pragma once

#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Containers/ByteView.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/EnumFlags.h>

namespace ngine::Reflection
{
	struct Registry;
	struct TypeDefinition;

	namespace Internal
	{
		enum class Operation
		{
			GetTypeSize,
			GetTypeAlignment,
			GetTypeName,
			GetPointerTypeDefinition,
			GetValueTypeDefinition,
			IsTriviallyCopyable,

			// Reference functions
			AreReferencesEqual,
			GetReferenceByteView,
			ReferencePlacementNewDefaultConstruct,
			ReferencePlacementNewCopyConstruct,
			ReferencePlacementNewMoveConstruct,

			// Stored object functions
			GetStoredObjectReference,
			StoredObjectDestroy,
			AreStoredObjectsEqual,
			StoredObjectPlacementNewDefaultConstruct,
			StoredObjectPlacementNewCopyConstruct,
			StoredObjectPlacementNewMoveConstruct,
			SerializeStoredObject,
			DeserializeStoredObject,
			DeserializeConstructStoredObject
		};

		struct OperationArgument
		{
			const TypeDefinition& m_type;
			union
			{
				Array<void*, 2> m_typeArgs;
				struct
				{
					void* pObject;
					Array<void*, 1> m_args;
				} m_objectReferenceArgs;
				struct
				{
					void* pObject;
					Array<void*, 2> m_args;
				} m_storedObjectArgs;
			};
		};

		template<typename Type>
		struct GenericTypeDefinition
		{
			[[nodiscard]] static Type& GetStoredObject(void* pAddress)
			{
				uintptr address = reinterpret_cast<uintptr>(pAddress);
				address += address % alignof(Type);
				return *reinterpret_cast<Type*>(address);
			}

			static void Manage(const Operation operation, OperationArgument& argument);
		};

		template<>
		struct GenericTypeDefinition<void>
		{
			static void Manage(const Operation operation, OperationArgument& argument);
		};
	}

	struct TypeDefinition
	{
	protected:
		using Operation = Internal::Operation;
		using OperationArgument = Internal::OperationArgument;

		using ReferenceType = void*;
		using ConstReferenceType = const void*;
		using StoredType = void*;
		using ConstStoredType = const void*;
	public:
		constexpr TypeDefinition() = default;

		template<typename Type>
		[[nodiscard]] static constexpr TypeDefinition Get()
		{
			return TypeDefinition{&Internal::GenericTypeDefinition<Type>::Manage};
		}

		template<typename Type>
		[[nodiscard]] constexpr bool Is() const
		{
			return *this == TypeDefinition::Get<Type>();
		}

		[[nodiscard]] constexpr bool IsValid() const
		{
			return m_manager != Internal::GenericTypeDefinition<void>::Manage;
		}

		[[nodiscard]] constexpr bool IsInvalid() const
		{
			return m_manager == Internal::GenericTypeDefinition<void>::Manage;
		}

		[[nodiscard]] constexpr bool operator==(const TypeDefinition& other) const
		{
			return m_manager == other.m_manager;
		}

		[[nodiscard]] constexpr bool operator!=(const TypeDefinition& other) const
		{
			return m_manager != other.m_manager;
		}

		[[nodiscard]] uint32 GetSize() const;
		[[nodiscard]] uint16 GetAlignment() const;
		[[nodiscard]] ConstUnicodeStringView GetTypeName() const;
		[[nodiscard]] bool IsTriviallyCopyable() const;

		[[nodiscard]] TypeDefinition GetPointerTypeDefinition() const;
		[[nodiscard]] TypeDefinition GetValueTypeDefinition() const;

		[[nodiscard]] bool AreReferencesEqual(const ConstReferenceType a, const ConstReferenceType b) const;

		[[nodiscard]] ByteView GetByteViewAtAddress(const ReferenceType address) const;

		void DefaultConstructAt(void* pAddress) const;
		void CopyConstructAt(void* pAddress, const void* pSourceAddress) const;
		void MoveConstructAt(void* pAddress, void* pSourceAddress) const;

		[[nodiscard]] void* GetAlignedObject(void* pUnalignedAddress) const;
		void DestroyUnalignedObject(const void* pUnalignedAddress) const;

		[[nodiscard]] bool AreStoredObjectsEqual(const ConstStoredType a, const ConstStoredType b) const;

		[[nodiscard]] uint32 TryDefaultConstructStoredObject(const StoredType target, uint32 targetCapacity) const;
		[[nodiscard]] uint32 TryCopyConstructStoredObject(const StoredType target, uint32 targetCapacity, const ConstStoredType source) const;
		[[nodiscard]] uint32 TryMoveConstructStoredObject(const StoredType target, uint32 targetCapacity, const StoredType source) const;

		bool SerializeStoredObject(const ConstStoredType target, Serialization::Writer&) const;
		bool SerializeStoredObject(const StoredType source, const Serialization::Reader&) const;
		bool SerializeConstructStoredObject(const StoredType source, const Serialization::Reader&) const;

		struct Hash
		{
			size operator()(const TypeDefinition& typeDefinition) const;
		};

		[[nodiscard]] void* GetUserData() const
		{
			return m_pUserData;
		}
	protected:
		using ManagerFunction = void (*)(const Operation, OperationArgument&);

		constexpr TypeDefinition(ManagerFunction manager, void* pUserData = nullptr)
			: m_manager(manager)
			, m_pUserData(pUserData)
		{
		}

		ManagerFunction m_manager = &Internal::GenericTypeDefinition<void>::Manage;
		void* m_pUserData = nullptr;
	};
}

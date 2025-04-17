#pragma once

#include <Common/Guid.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/ForwardDeclarations/Any.h>
#include <Common/Function/FunctionPointer.h>
#include <Common/Function/Function.h>
#include <Common/EnumFlags.h>
#include <Common/EnumFlagOperators.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Reflection/TypeInitializer.h>
#include <Common/Reflection/TypeDeserializer.h>
#include <Common/Reflection/TypeCloner.h>
#include <Common/Reflection/Extension.h>
#include <Common/Reflection/GetType.h>
#include <Common/Tag/TagGuid.h>

namespace ngine
{
	struct ConstAnyView;
}

namespace ngine::Threading
{
	struct JobBatch;
}

namespace ngine::Reflection
{
	struct PropertyInfo;
	struct FunctionInfo;
	struct EventInfo;
	struct DynamicPropertyInstance;
	struct TypeDefinition;
	struct DynamicallyConstructibleType;
	struct Registry;
	struct ExtensionInterface;

	enum class TypeFlags : uint16
	{
		//! Disable support for instantiating this type at runtime from UI
		DisableUserInterfaceInstantiation = 1 << 0,
		//! Whether this type should be removable from the UI
		DisableDeletionFromUserInterface = 1 << 1,
		// Disables support for instantiating this type at runtime without knowing the type info
		DisableDynamicInstantiation = 1 << 2,
		// Disables support for cloning this type at runtime without knowing the type info
		DisableDynamicCloning = 1 << 3,
		// Disables support for deserializing this type at runtime without knowing the type info
		DisableDynamicDeserialization = 1 << 4,

		DisableWriteToDisk = 1 << 5,

		IsAbstract = 1 << 6,

		HasProperties = 1 << 7,
		HasInheritedProperties = 1 << 8,

		HasNetworkedProperties = 1 << 9,
		HasNetworkedFunctions = 1 << 10,
		IsNetworked = HasNetworkedProperties | HasNetworkedFunctions
	};

	ENUM_FLAG_OPERATORS(TypeFlags);

	struct TypeInterface
	{
		constexpr TypeInterface() = default;

		constexpr TypeInterface(
			const Guid typeGuid,
			const ConstUnicodeStringView name,
			const ConstUnicodeStringView description,
			const EnumFlags<TypeFlags> flags,
			const TypeDefinition& typeDefinition,
			const TypeInterface* pParent = nullptr
		)
			: m_guid(typeGuid)
			, m_name(name)
			, m_description(description)
			, m_flags(flags)
			, m_typeDefinition(typeDefinition)
			, m_pParent(pParent)
		{
		}

		[[nodiscard]] constexpr Guid GetGuid() const
		{
			return m_guid;
		}

		[[nodiscard]] constexpr ConstUnicodeStringView GetName() const
		{
			return m_name;
		}

		[[nodiscard]] constexpr ConstUnicodeStringView GetDescription() const
		{
			return m_description;
		}

		[[nodiscard]] constexpr EnumFlags<TypeFlags> GetFlags() const
		{
			return m_flags;
		}

		[[nodiscard]] constexpr bool IsAbstract() const
		{
			return m_flags.IsSet(TypeFlags::IsAbstract);
		}

		[[nodiscard]] constexpr const TypeDefinition GetTypeDefinition() const
		{
			return m_typeDefinition;
		}

		[[nodiscard]] constexpr const TypeInterface* GetParent() const
		{
			return m_pParent;
		}

		template<typename Callback>
		constexpr void IterateHierarchy(Callback&& callback) const
		{
			const TypeInterface* pTypeInterface = this;
			do
			{
				callback(*pTypeInterface);
				pTypeInterface = pTypeInterface->GetParent();
			} while (pTypeInterface != nullptr);
		}

		using PropertyQueryResult = Tuple<const PropertyInfo*, DynamicPropertyInstance>;
		[[nodiscard]] virtual PropertyQueryResult FindProperty(const Guid propertyGuid) const = 0;

		using IteratePropertiesCallback = ngine::Function<void(const PropertyInfo&, DynamicPropertyInstance&&), 24>;
		constexpr void IterateProperties(IteratePropertiesCallback&& callback) const
		{
			m_iteratePropertiesFunction(*this, Forward<IteratePropertiesCallback>(callback));
		}

		[[nodiscard]] virtual Optional<const FunctionInfo*> FindFunction(const Guid functionGuid) const = 0;

		using IterateFunctionsCallback = ngine::Function<void(const FunctionInfo&), 24>;
		constexpr void IterateFunctions(IterateFunctionsCallback&& callback) const
		{
			m_iterateFunctionsFunction(*this, Forward<IterateFunctionsCallback>(callback));
		}

		[[nodiscard]] virtual Optional<const EventInfo*> FindEvent(const Guid eventGuid) const = 0;

		using IterateEventsCallback = ngine::Function<void(const EventInfo&), 24>;
		constexpr void IterateEvents(IterateEventsCallback&& callback) const
		{
			m_iterateEventsFunction(*this, Forward<IterateEventsCallback>(callback));
		}

		[[nodiscard]] virtual ArrayView<const Tag::Guid> GetTags() const = 0;

		[[nodiscard]] PURE_STATICS constexpr bool Implements(const Guid typeGuid) const
		{
			const TypeInterface* pTypeInterface = this;
			do
			{
				if (pTypeInterface->GetGuid() == typeGuid)
				{
					return true;
				}
				pTypeInterface = pTypeInterface->GetParent();
			} while (pTypeInterface != nullptr);
			return false;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr bool Implements() const
		{
			return Implements(Reflection::GetTypeGuid<Type>());
		}

		template<typename ExtensionType>
		[[nodiscard]] Optional<const ExtensionType*> FindExtension() const
		{
			return static_cast<const ExtensionType*>(FindExtension(ExtensionType::TypeGuid).Get());
		}
		[[nodiscard]] virtual Optional<const ExtensionInterface*> FindExtension(const Guid typeGuid) const = 0;
	protected:
		Guid m_guid;
		ConstUnicodeStringView m_name;
		ConstUnicodeStringView m_description;
		EnumFlags<TypeFlags> m_flags;
		TypeDefinition m_typeDefinition;
		const TypeInterface* m_pParent = nullptr;
		FunctionPointer<void(const TypeInterface&, IteratePropertiesCallback&&)> m_iteratePropertiesFunction;
		FunctionPointer<void(const TypeInterface&, IterateFunctionsCallback&&)> m_iterateFunctionsFunction;
		FunctionPointer<void(const TypeInterface&, IterateEventsCallback&&)> m_iterateEventsFunction;
	};
};

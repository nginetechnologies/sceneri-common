#pragma once

#include "ForwardDeclarations/Type.h"

#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Guid.h>
#include <Common/Reflection/TypeDefinition.h>
#include <Common/Reflection/FunctionFlags.h>
#include <Common/Function/ForwardDeclarations/Function.h>
#include <Common/Scripting/VirtualMachine/DynamicFunction/DynamicFunction.h>
#include <Common/Scripting/VirtualMachine/FunctionIdentifier.h>
#include <Common/Platform/Pure.h>
#include <Common/Memory/CallbackResult.h>
#include <Common/Threading/Mutexes/SharedMutex.h>
#include <Common/System/SystemType.h>
#include <Common/EnumFlags.h>
#include <Common/Storage/ForwardDeclarations/IdentifierMask.h>
#include <Common/Storage/ForwardDeclarations/AtomicIdentifierMask.h>
#include <Common/Storage/SaltedIdentifierStorage.h>
#include <Common/Storage/IdentifierArray.h>

namespace ngine::Reflection
{
	struct TypeInterface;
	struct FunctionInfo;
	struct EventInfo;

	struct FunctionData
	{
		[[nodiscard]] bool IsValid() const
		{
			return m_function.IsValid();
		}

		Scripting::VM::DynamicFunction m_function;
		EnumFlags<FunctionFlags> m_flags;
		Guid m_owningTypeGuid;
	};
}

namespace ngine
{
	extern template struct UnorderedMap<Guid, const Reflection::TypeInterface*, Guid::Hash>;
	extern template struct UnorderedMap<Guid, Reflection::TypeDefinition, Guid::Hash>;
	extern template struct UnorderedMap<Guid, Reflection::FunctionData, Guid::Hash>;
}

namespace ngine::Reflection
{
	using FunctionIdentifier = Scripting::FunctionIdentifier;
	using FunctionMask = IdentifierMask<FunctionIdentifier>;
	using AtomicFunctionMask = Threading::AtomicIdentifierMask<FunctionIdentifier>;

	struct Registry
	{
		enum class Initializer
		{
			Initialize
		};

		inline static constexpr System::Type SystemType = System::Type::Reflection;

		Registry() = default;
		Registry(const Initializer);
		~Registry();

		[[nodiscard]] uint16 GetTypeCount() const
		{
			Threading::SharedLock lock(m_typeMutex);
			return (uint16)m_types.GetSize();
		}

		static bool RegisterType(const Guid guid, const TypeInterface& typeInterface);
		template<typename Type>
		static bool RegisterType();
		static bool RegisterTypeDefinition(const Guid guid, TypeDefinition&& definition);

		template<auto Function>
		static bool RegisterGlobalFunction();
		static bool RegisterFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid);
		static bool RegisterGlobalFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function);

		template<auto Event>
		static bool RegisterGlobalEvent();
		static bool RegisterEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid);
		static bool RegisterGlobalEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function);

		void RegisterDynamicType(const Guid guid, const TypeInterface& typeInterface);
		template<typename Type>
		void RegisterDynamicType();
		void DeregisterDynamicType(const Guid guid);
		template<typename Type>
		void DeregisterDynamicType();

		void RegisterDynamicTypeDefinition(const Guid guid, TypeDefinition&& definition);
		void DeregisterDynamicTypeDefinition(const Guid guid);

		FunctionIdentifier
		RegisterDynamicFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid);
		void DeregisterDynamicFunction(const Guid guid);
		FunctionIdentifier RegisterDynamicGlobalFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function);
		void DeregisterDynamicGlobalFunction(const Guid guid);

		template<auto Function>
		void RegisterDynamicGlobalFunction();
		template<auto Function>
		void DeregisterDynamicGlobalFunction();

		[[nodiscard]] PURE_STATICS const FunctionData& FindFunction(const Guid guid) const;
		[[nodiscard]] PURE_STATICS Optional<const FunctionInfo*> FindGlobalFunctionDefinition(const Guid guid) const;
		[[nodiscard]] PURE_STATICS Optional<const FunctionInfo*> FindTypeFunctionDefinition(const Guid guid) const;
		[[nodiscard]] PURE_STATICS Optional<const FunctionInfo*> FindFunctionDefinition(const Guid guid) const;

		[[nodiscard]] PURE_STATICS Guid FindFunctionGuid(const FunctionIdentifier identifier) const;
		[[nodiscard]] PURE_STATICS FunctionIdentifier FindFunctionIdentifier(const Guid guid) const;

		using IterateGlobalFunctionsCallback = ngine::Function<Memory::CallbackResult(const FunctionInfo&), 32>;
		void IterateGlobalFunctions(IterateGlobalFunctionsCallback&& callback) const;

		FunctionIdentifier
		RegisterDynamicEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid);
		void DeregisterDynamicEvent(const Guid guid);
		FunctionIdentifier RegisterDynamicGlobalEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function);
		void DeregisterDynamicGlobalEvent(const Guid guid);

		template<auto Event>
		void RegisterDynamicGlobalEvent();
		template<auto Event>
		void DeregisterDynamicGlobalEvent();

		[[nodiscard]] PURE_STATICS const FunctionData& FindEvent(const Guid guid) const
		{
			return FindFunction(guid);
		}
		[[nodiscard]] PURE_STATICS Optional<const EventInfo*> FindGlobalEventDefinition(const Guid guid) const;
		[[nodiscard]] PURE_STATICS Optional<const EventInfo*> FindTypeEventDefinition(const Guid guid) const;
		[[nodiscard]] PURE_STATICS Optional<const EventInfo*> FindEventDefinition(const Guid guid) const;

		[[nodiscard]] PURE_STATICS Guid FindEventGuid(const FunctionIdentifier identifier) const
		{
			return FindFunctionGuid(identifier);
		}
		[[nodiscard]] PURE_STATICS FunctionIdentifier FindEventIdentifier(const Guid guid) const
		{
			return FindFunctionIdentifier(guid);
		}

		using IterateGlobalEventsCallback = ngine::Function<Memory::CallbackResult(const EventInfo&), 32>;
		void IterateGlobalEvents(IterateGlobalEventsCallback&& callback) const;

		[[nodiscard]] PURE_STATICS Optional<const TypeDefinition*> FindTypeDefinition(const Guid guid) const;
		[[nodiscard]] PURE_STATICS Guid FindTypeDefinitionGuid(const TypeDefinition definition) const;
		[[nodiscard]] PURE_STATICS Optional<const TypeInterface*> FindTypeInterface(const Guid guid) const;

		using IterateTypeInterfacesCallback = ngine::Function<Memory::CallbackResult(const TypeInterface&), 32>;
		void IterateTypeInterfaces(IterateTypeInterfacesCallback&& callback) const;
	protected:
		mutable Threading::SharedMutex m_typeMutex;
		using TypeContainer = UnorderedMap<Guid, const TypeInterface*, Guid::Hash>;
		TypeContainer m_types;
		mutable Threading::SharedMutex m_typeDefinitionMutex;
		UnorderedMap<Guid, TypeDefinition, Guid::Hash> m_typeDefinitions;

		mutable Threading::SharedMutex m_functionMutex;
		using FunctionContainer = UnorderedMap<Guid, FunctionData, Guid::Hash>;
		FunctionContainer m_functions;
		using GlobalFunctionContainer = UnorderedMap<Guid, const FunctionInfo*, Guid::Hash>;
		GlobalFunctionContainer m_globalFunctions;
		TSaltedIdentifierStorage<FunctionIdentifier> m_functionIdentifiers;
		using FunctionLookupContainer = TIdentifierArray<Guid, FunctionIdentifier>;
		FunctionLookupContainer m_functionGuids{Memory::Zeroed};
		UnorderedMap<Guid, FunctionIdentifier, Guid::Hash> m_functionIdentifierLookupMap;

		using GlobalEventContainer = UnorderedMap<Guid, const EventInfo*, Guid::Hash>;
		GlobalEventContainer m_globalEvents;
	};
}

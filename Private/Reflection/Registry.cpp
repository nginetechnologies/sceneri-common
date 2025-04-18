#include "Reflection/Registry.h"

#include <Common/Reflection/TypeInterface.h>
#include <Common/Reflection/FunctionInfo.h>

namespace ngine::Reflection
{
	/* static */ Registry& GetStaticRegistry()
	{
		static Registry storage;
		return storage;
	}

	/*static*/ bool Registry::RegisterType(const Guid guid, const TypeInterface& typeInterface)
	{
		Registry& staticRegistry = GetStaticRegistry();
		staticRegistry.RegisterDynamicType(guid, typeInterface);
		return true;
	}

	/*static*/ bool Registry::RegisterTypeDefinition(const Guid guid, TypeDefinition&& definition)
	{
		Registry& staticRegistry = GetStaticRegistry();
		staticRegistry.RegisterDynamicTypeDefinition(guid, Forward<TypeDefinition>(definition));
		return true;
	}

	/*static*/ bool
	Registry::RegisterFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid)
	{
		Registry& staticRegistry = GetStaticRegistry();
		staticRegistry.RegisterDynamicFunction(functionInfo, Forward<Scripting::VM::DynamicFunction>(function), owningTypeGuid);
		return true;
	}

	/*static*/ bool Registry::RegisterGlobalFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function)
	{
		Registry& staticRegistry = GetStaticRegistry();
		staticRegistry.RegisterDynamicGlobalFunction(functionInfo, Forward<Scripting::VM::DynamicFunction>(function));
		return true;
	}

	/*static*/ bool Registry::RegisterEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid)
	{
		Registry& staticRegistry = GetStaticRegistry();
		staticRegistry.RegisterDynamicEvent(eventInfo, Forward<Scripting::VM::DynamicFunction>(function), owningTypeGuid);
		return true;
	}

	/*static*/ bool Registry::RegisterGlobalEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function)
	{
		Registry& staticRegistry = GetStaticRegistry();
		staticRegistry.RegisterDynamicGlobalEvent(eventInfo, Forward<Scripting::VM::DynamicFunction>(function));
		return true;
	}

	Registry::Registry(const Initializer)
	{
		Registry& staticRegistry = GetStaticRegistry();
		m_types = staticRegistry.m_types;
		m_typeDefinitions = staticRegistry.m_typeDefinitions;
		m_functions = staticRegistry.m_functions;
		m_globalFunctions = staticRegistry.m_globalFunctions;
		m_globalEvents = staticRegistry.m_globalEvents;

		// Register functions
		m_functionIdentifierLookupMap.Reserve(m_functions.GetSize());
		for (const Registry::FunctionContainer::PairType& functionPair : m_functions)
		{
			const FunctionIdentifier identifier = m_functionIdentifiers.AcquireIdentifier();
			m_functionGuids[identifier] = functionPair.first;
			m_functionIdentifierLookupMap.Emplace(Guid{functionPair.first}, FunctionIdentifier{identifier});
		}
	}

	Registry::~Registry() = default;

	void Registry::RegisterDynamicType(const Guid guid, const TypeInterface& typeInterface)
	{
		{
			Threading::UniqueLock lock(m_typeMutex);
			Assert(!m_types.Contains(guid));
			m_types.Emplace(Guid(guid), &typeInterface);
		}
		RegisterDynamicTypeDefinition(guid, TypeDefinition(typeInterface.GetTypeDefinition()));
	}

	void Registry::DeregisterDynamicType(const Guid guid)
	{
		{
			Threading::UniqueLock lock(m_typeMutex);
			auto it = m_types.Find(guid);
			Assert(it != m_types.end());
			if (LIKELY(it != m_types.end()))
			{
				m_types.Remove(it);
			}
		}
		DeregisterDynamicTypeDefinition(guid);
	}

	void Registry::RegisterDynamicTypeDefinition(const Guid guid, TypeDefinition&& definition)
	{
		Threading::UniqueLock lock(m_typeDefinitionMutex);
		Assert(!m_typeDefinitions.Contains(guid));
		m_typeDefinitions.Emplace(Guid(guid), Forward<TypeDefinition>(definition));
	}

	void Registry::DeregisterDynamicTypeDefinition(const Guid guid)
	{
		Threading::UniqueLock lock(m_typeDefinitionMutex);
		auto it = m_typeDefinitions.Find(guid);
		Assert(it != m_typeDefinitions.end());
		if (LIKELY(it != m_typeDefinitions.end()))
		{
			m_typeDefinitions.Remove(it);
		}
	}

	FunctionIdentifier
	Registry::RegisterDynamicFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid)
	{
		Threading::UniqueLock lock(m_functionMutex);
		Assert(!m_functions.Contains(functionInfo.m_guid));
		m_functions.Emplace(
			Guid(functionInfo.m_guid),
			FunctionData{Forward<Scripting::VM::DynamicFunction>(function), functionInfo.m_flags, owningTypeGuid}
		);

		const FunctionIdentifier identifier = m_functionIdentifiers.AcquireIdentifier();
		m_functionGuids[identifier] = functionInfo.m_guid;
		m_functionIdentifierLookupMap.Emplace(Guid{functionInfo.m_guid}, FunctionIdentifier{identifier});
		return identifier;
	}

	void Registry::DeregisterDynamicFunction(const Guid guid)
	{
		Threading::UniqueLock lock(m_functionMutex);
		{
			auto it = m_functions.Find(guid);
			Assert(it != m_functions.end());
			if (LIKELY(it != m_functions.end()))
			{
				m_functions.Remove(it);
			}
		}
		{
			auto it = m_functionIdentifierLookupMap.Find(guid);
			Assert(it != m_functionIdentifierLookupMap.end());
			if (LIKELY(it != m_functionIdentifierLookupMap.end()))
			{
				m_functionIdentifierLookupMap.Remove(it);
			}
		}
	}

	FunctionIdentifier Registry::RegisterDynamicGlobalFunction(const FunctionInfo& functionInfo, Scripting::VM::DynamicFunction&& function)
	{
		Threading::UniqueLock lock(m_functionMutex);
		Assert(!m_functions.Contains(functionInfo.m_guid));
		m_functions
			.Emplace(Guid(functionInfo.m_guid), FunctionData{Forward<Scripting::VM::DynamicFunction>(function), functionInfo.m_flags, {}});
		m_globalFunctions.Emplace(Guid(functionInfo.m_guid), &functionInfo);

		const FunctionIdentifier identifier = m_functionIdentifiers.AcquireIdentifier();
		m_functionGuids[identifier] = functionInfo.m_guid;
		m_functionIdentifierLookupMap.Emplace(Guid{functionInfo.m_guid}, FunctionIdentifier{identifier});
		return identifier;
	}

	void Registry::DeregisterDynamicGlobalFunction(const Guid guid)
	{
		Threading::UniqueLock lock(m_functionMutex);
		{
			auto it = m_functions.Find(guid);
			Assert(it != m_functions.end());
			if (LIKELY(it != m_functions.end()))
			{
				m_functions.Remove(it);
			}
		}
		{
			auto it = m_globalFunctions.Find(guid);
			Assert(it != m_globalFunctions.end());
			if (LIKELY(it != m_globalFunctions.end()))
			{
				m_globalFunctions.Remove(it);
			}
		}
		{
			auto it = m_functionIdentifierLookupMap.Find(guid);
			Assert(it != m_functionIdentifierLookupMap.end());
			if (LIKELY(it != m_functionIdentifierLookupMap.end()))
			{
				m_functionIdentifierLookupMap.Remove(it);
			}
		}
	}

	PURE_STATICS const FunctionData& Registry::FindFunction(const Guid guid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_functions.Find(guid); it != m_functions.end())
		{
			return it->second;
		}
		static FunctionData dummyFunctionData;
		return dummyFunctionData;
	}

	PURE_STATICS Optional<const FunctionInfo*> Registry::FindGlobalFunctionDefinition(const Guid guid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_globalFunctions.Find(guid); it != m_globalFunctions.end())
		{
			return it->second;
		}
		return Invalid;
	}

	PURE_STATICS Optional<const FunctionInfo*> Registry::FindTypeFunctionDefinition(const Guid functionGuid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_functions.Find(functionGuid); it != m_functions.end())
		{
			if (it->second.m_flags.IsNotSet(FunctionFlags::IsEvent))
			{
				Assert(it->second.m_flags.IsSet(FunctionFlags::IsMemberFunction));
				Assert(it->second.m_owningTypeGuid.IsValid());

				const Optional<const TypeInterface*> pOwningTypeInterface = FindTypeInterface(it->second.m_owningTypeGuid);
				Optional<const Reflection::FunctionInfo*> pFunctionInfo;
				Assert(pOwningTypeInterface.IsValid());
				if (LIKELY(pOwningTypeInterface.IsValid()))
				{
					pOwningTypeInterface->IterateFunctions(
						[functionGuid, &pFunctionInfo](const Reflection::FunctionInfo& functionInfo)
						{
							if (functionInfo.m_guid == functionGuid)
							{
								pFunctionInfo = &functionInfo;
							}
						}
					);
				}
				return pFunctionInfo;
			}
		}
		return Invalid;
	}

	PURE_STATICS Optional<const FunctionInfo*> Registry::FindFunctionDefinition(const Guid functionGuid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_functions.Find(functionGuid); it != m_functions.end())
		{
			if (it->second.m_flags.IsNotSet(FunctionFlags::IsEvent))
			{
				Assert(it->second.m_flags.IsSet(FunctionFlags::IsMemberFunction) == it->second.m_owningTypeGuid.IsValid());
				if (it->second.m_flags.IsSet(FunctionFlags::IsMemberFunction))
				{
					const Optional<const TypeInterface*> pOwningTypeInterface = FindTypeInterface(it->second.m_owningTypeGuid);
					Optional<const Reflection::FunctionInfo*> pFunctionInfo;
					Assert(pOwningTypeInterface.IsValid());
					if (LIKELY(pOwningTypeInterface.IsValid()))
					{
						pOwningTypeInterface->IterateFunctions(
							[functionGuid, &pFunctionInfo](const Reflection::FunctionInfo& functionInfo)
							{
								if (functionInfo.m_guid == functionGuid)
								{
									pFunctionInfo = &functionInfo;
								}
							}
						);
					}
					return pFunctionInfo;
				}
				else if (auto globalIt = m_globalFunctions.Find(functionGuid); globalIt != m_globalFunctions.end())
				{
					return globalIt->second;
				}
			}
		}
		return Invalid;
	}

	void Registry::IterateGlobalFunctions(IterateGlobalFunctionsCallback&& callback) const
	{
		Threading::SharedLock lock(m_functionMutex);
		for (const GlobalFunctionContainer::PairType& functionPair : m_globalFunctions)
		{
			if (callback(*functionPair.second) == Memory::CallbackResult::Break)
			{
				return;
			}
		}
	}

	Guid Registry::FindFunctionGuid(const FunctionIdentifier identifier) const
	{
		return m_functionGuids[identifier];
	}

	FunctionIdentifier Registry::FindFunctionIdentifier(const Guid guid) const
	{
		auto it = m_functionIdentifierLookupMap.Find(guid);
		if (it != m_functionIdentifierLookupMap.end())
		{
			return it->second;
		}
		return {};
	}

	FunctionIdentifier
	Registry::RegisterDynamicEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function, const Guid owningTypeGuid)
	{
		Threading::UniqueLock lock(m_functionMutex);
		Assert(!m_functions.Contains(eventInfo.m_guid));
		m_functions.Emplace(
			Guid(eventInfo.m_guid),
			FunctionData{Forward<Scripting::VM::DynamicFunction>(function), FunctionFlags::IsEvent, owningTypeGuid}
		);

		const FunctionIdentifier identifier = m_functionIdentifiers.AcquireIdentifier();
		m_functionGuids[identifier] = eventInfo.m_guid;
		m_functionIdentifierLookupMap.Emplace(Guid{eventInfo.m_guid}, FunctionIdentifier{identifier});
		return identifier;
	}

	void Registry::DeregisterDynamicEvent(const Guid guid)
	{
		Threading::UniqueLock lock(m_functionMutex);
		{
			auto it = m_functions.Find(guid);
			Assert(it != m_functions.end());
			if (LIKELY(it != m_functions.end()))
			{
				m_functions.Remove(it);
			}
		}
		{
			auto it = m_functionIdentifierLookupMap.Find(guid);
			Assert(it != m_functionIdentifierLookupMap.end());
			if (LIKELY(it != m_functionIdentifierLookupMap.end()))
			{
				m_functionIdentifierLookupMap.Remove(it);
			}
		}
	}

	FunctionIdentifier Registry::RegisterDynamicGlobalEvent(const EventInfo& eventInfo, Scripting::VM::DynamicFunction&& function)
	{
		Threading::UniqueLock lock(m_functionMutex);
		Assert(!m_functions.Contains(eventInfo.m_guid));
		m_functions
			.Emplace(Guid(eventInfo.m_guid), FunctionData{Forward<Scripting::VM::DynamicFunction>(function), FunctionFlags::IsEvent, {}});
		m_globalEvents.Emplace(Guid(eventInfo.m_guid), &eventInfo);

		const FunctionIdentifier identifier = m_functionIdentifiers.AcquireIdentifier();
		m_functionGuids[identifier] = eventInfo.m_guid;
		m_functionIdentifierLookupMap.Emplace(Guid{eventInfo.m_guid}, FunctionIdentifier{identifier});
		return identifier;
	}

	void Registry::DeregisterDynamicGlobalEvent(const Guid guid)
	{
		Threading::UniqueLock lock(m_functionMutex);
		{
			auto it = m_functions.Find(guid);
			Assert(it != m_functions.end());
			if (LIKELY(it != m_functions.end()))
			{
				m_functions.Remove(it);
			}
		}
		{
			auto it = m_globalFunctions.Find(guid);
			Assert(it != m_globalFunctions.end());
			if (LIKELY(it != m_globalFunctions.end()))
			{
				m_globalFunctions.Remove(it);
			}
		}
		{
			auto it = m_functionIdentifierLookupMap.Find(guid);
			Assert(it != m_functionIdentifierLookupMap.end());
			if (LIKELY(it != m_functionIdentifierLookupMap.end()))
			{
				m_functionIdentifierLookupMap.Remove(it);
			}
		}
	}

	PURE_STATICS Optional<const EventInfo*> Registry::FindGlobalEventDefinition(const Guid guid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_globalEvents.Find(guid); it != m_globalEvents.end())
		{
			return it->second;
		}
		return Invalid;
	}

	PURE_STATICS Optional<const EventInfo*> Registry::FindTypeEventDefinition(const Guid eventGuid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_functions.Find(eventGuid); it != m_functions.end())
		{
			if (it->second.m_flags.IsSet(FunctionFlags::IsEvent))
			{
				Assert(it->second.m_owningTypeGuid.IsValid());
				const Optional<const TypeInterface*> pOwningTypeInterface = FindTypeInterface(it->second.m_owningTypeGuid);
				Optional<const Reflection::EventInfo*> pEventInfo;
				Assert(pOwningTypeInterface.IsValid());
				if (LIKELY(pOwningTypeInterface.IsValid()))
				{
					pOwningTypeInterface->IterateEvents(
						[eventGuid, &pEventInfo](const Reflection::EventInfo& eventInfo)
						{
							if (eventInfo.m_guid == eventGuid)
							{
								pEventInfo = &eventInfo;
							}
						}
					);
				}
				return pEventInfo;
			}
		}
		return Invalid;
	}

	PURE_STATICS Optional<const EventInfo*> Registry::FindEventDefinition(const Guid eventGuid) const
	{
		Threading::SharedLock lock(m_functionMutex);
		if (auto it = m_functions.Find(eventGuid); it != m_functions.end())
		{
			if (it->second.m_flags.IsSet(FunctionFlags::IsEvent))
			{
				if (it->second.m_owningTypeGuid.IsValid())
				{
					Optional<const Reflection::EventInfo*> pEventInfo;
					const Optional<const TypeInterface*> pOwningTypeInterface = FindTypeInterface(it->second.m_owningTypeGuid);
					Assert(pOwningTypeInterface.IsValid());
					if (LIKELY(pOwningTypeInterface.IsValid()))
					{
						pOwningTypeInterface->IterateEvents(
							[eventGuid, &pEventInfo](const Reflection::EventInfo& eventInfo)
							{
								if (eventInfo.m_guid == eventGuid)
								{
									pEventInfo = &eventInfo;
								}
							}
						);
					}
					return pEventInfo;
				}
				else if (auto globalIt = m_globalEvents.Find(eventGuid); globalIt != m_globalEvents.end())
				{
					return globalIt->second;
				}
			}
		}
		return Invalid;
	}

	void Registry::IterateGlobalEvents(IterateGlobalEventsCallback&& callback) const
	{
		Threading::SharedLock lock(m_functionMutex);
		for (const GlobalEventContainer::PairType& eventPair : m_globalEvents)
		{
			if (callback(*eventPair.second) == Memory::CallbackResult::Break)
			{
				return;
			}
		}
	}

	PURE_STATICS Optional<const TypeDefinition*> Registry::FindTypeDefinition(const Guid guid) const
	{
		Threading::SharedLock lock(m_typeDefinitionMutex);
		auto it = m_typeDefinitions.Find(guid);
		if (it != m_typeDefinitions.end())
		{
			return it->second;
		}
		return Invalid;
	}

	PURE_STATICS Guid Registry::FindTypeDefinitionGuid(const TypeDefinition definition) const
	{
		Threading::SharedLock lock(m_typeDefinitionMutex);
		for (auto it = m_typeDefinitions.begin(), end = m_typeDefinitions.end(); it != end; ++it)
		{
			if (it->second == definition)
			{
				return it->first;
			}
		}

		return {};
	}

	PURE_STATICS Optional<const TypeInterface*> Registry::FindTypeInterface(const Guid guid) const
	{
		Threading::SharedLock lock(m_typeMutex);
		auto it = m_types.Find(guid);
		if (it != m_types.end())
		{
			return it->second;
		}
		return Invalid;
	}

	void Registry::IterateTypeInterfaces(IterateTypeInterfacesCallback&& callback) const
	{
		Threading::SharedLock lock(m_typeMutex);
		for (const TypeContainer::PairType& typePair : m_types)
		{
			if (callback(*typePair.second) == Memory::CallbackResult::Break)
			{
				return;
			}
		}
	}
}

namespace ngine
{
	template struct UnorderedMap<Guid, const Reflection::TypeInterface*, Guid::Hash>;
	template struct UnorderedMap<Guid, Reflection::TypeDefinition, Guid::Hash>;
	template struct UnorderedMap<Guid, Reflection::FunctionData, Guid::Hash>;
}

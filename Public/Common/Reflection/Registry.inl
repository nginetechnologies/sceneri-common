#pragma once

#include <Common/Reflection/Registry.h>
#include <Common/Reflection/GetType.h>
#include <Common/Scripting/VirtualMachine/DynamicFunction/DynamicFunction.h>
#include <Common/Reflection/GenericType.h>

namespace ngine::Reflection
{
	namespace Internal
	{
		template<size Index, typename Type>
		static void RegisterFunctions()
		{
			constexpr auto& reflectedType = GetType<Type>();
			using TupleType = decltype(reflectedType.m_functions);

			static constexpr auto& function = reflectedType.m_functions.template Get<Index>();

			Registry::RegisterFunction(function, Scripting::VM::DynamicFunction::Make<function.GetFunction()>(), reflectedType.GetGuid());

			if constexpr (Index < (TupleType::ElementCount - 1))
			{
				RegisterFunctions<Index + 1, Type>();
			}
		}

		template<size Index, typename Type>
		static void RegisterDynamicFunctions(Registry& registry)
		{
			constexpr auto& reflectedType = GetType<Type>();
			using TupleType = decltype(reflectedType.m_functions);

			static constexpr auto& function = reflectedType.m_functions.template Get<Index>();

			registry.RegisterDynamicFunction(function, Scripting::VM::DynamicFunction::Make<function.GetFunction()>(), reflectedType.GetGuid());

			if constexpr (Index < (TupleType::ElementCount - 1))
			{
				RegisterDynamicFunctions<Index + 1, Type>(registry);
			}
		}

		template<size Index, typename Type>
		static void DeregisterDynamicFunctions(Registry& registry)
		{
			constexpr auto& reflectedType = GetType<Type>();
			using TupleType = decltype(reflectedType.m_functions);

			static constexpr auto& function = reflectedType.m_functions.template Get<Index>();

			registry.DeregisterDynamicFunction(function.GetGuid());

			if constexpr (Index < (TupleType::ElementCount - 1))
			{
				DeregisterDynamicFunctions<Index + 1, Type>(registry);
			}
		}

		template<size Index, typename Type>
		static void RegisterEvents()
		{
			constexpr auto& reflectedType = GetType<Type>();
			using TupleType = decltype(reflectedType.m_events);

			static constexpr auto& event = reflectedType.m_events.template Get<Index>();

			if constexpr (event.IsGetterFunction)
			{
				Registry::RegisterEvent(event, Scripting::VM::DynamicFunction::Make<event.GetEvent()>(), reflectedType.GetGuid());
			}
			else
			{
				Registry::RegisterEvent(event, Scripting::VM::DynamicFunction::MakeGetVariable<event.GetEvent()>(), reflectedType.GetGuid());
			}

			if constexpr (Index < (TupleType::ElementCount - 1))
			{
				RegisterEvents<Index + 1, Type>();
			}
		}

		template<size Index, typename Type>
		static void RegisterDynamicEvents(Registry& registry)
		{
			constexpr auto& reflectedType = GetType<Type>();
			using TupleType = decltype(reflectedType.m_events);

			static constexpr auto& event = reflectedType.m_events.template Get<Index>();

			if constexpr (event.IsGetterFunction)
			{
				registry.RegisterDynamicEvent(event, Scripting::VM::DynamicFunction::Make<event.GetEvent()>(), reflectedType.GetGuid());
			}
			else
			{
				registry.RegisterDynamicEvent(event, Scripting::VM::DynamicFunction::MakeGetVariable<event.GetEvent()>(), reflectedType.GetGuid());
			}

			if constexpr (Index < (TupleType::ElementCount - 1))
			{
				RegisterDynamicEvents<Index + 1, Type>(registry);
			}
		}

		template<size Index, typename Type>
		static void DeregisterDynamicEvents(Registry& registry)
		{
			constexpr auto& reflectedType = GetType<Type>();
			using TupleType = decltype(reflectedType.m_events);

			static constexpr auto& event = reflectedType.m_events.template Get<Index>();

			registry.DeregisterDynamicEvent(event.GetGuid());

			if constexpr (Index < (TupleType::ElementCount - 1))
			{
				DeregisterDynamicEvents<Index + 1, Type>(registry);
			}
		}
	}

	template<typename Type>
	/*static*/ inline bool Registry::RegisterType()
	{
		static_assert(IsReflected<Type>, "Type was not reflected!");
		constexpr auto& reflectedType = GetType<Type>();
		if constexpr (reflectedType.HasFunctions)
		{
			Internal::RegisterFunctions<0, Type>();
		}
		if constexpr (reflectedType.HasEvents)
		{
			Internal::RegisterEvents<0, Type>();
		}

		return RegisterType(reflectedType.GetGuid(), reflectedType);
	}

	template<typename Type>
	inline void Registry::RegisterDynamicType()
	{
		static_assert(IsReflected<Type>, "Type was not reflected!");
		constexpr auto& reflectedType = GetType<Type>();
		if constexpr (reflectedType.HasFunctions)
		{
			Internal::RegisterDynamicFunctions<0, Type>(*this);
		}
		if constexpr (reflectedType.HasEvents)
		{
			Internal::RegisterDynamicEvents<0, Type>(*this);
		}

		RegisterDynamicType(reflectedType.GetGuid(), reflectedType);
	}

	template<typename Type>
	inline void Registry::DeregisterDynamicType()
	{
		static_assert(IsReflected<Type>, "Type was not reflected!");
		constexpr auto& reflectedType = GetType<Type>();
		if constexpr (reflectedType.HasFunctions)
		{
			Internal::DeregisterDynamicFunctions<0, Type>(*this);
		}
		if constexpr (reflectedType.HasEvents)
		{
			Internal::DeregisterDynamicEvents<0, Type>(*this);
		}

		DeregisterDynamicType(reflectedType.GetGuid());
	}

	template<auto Function>
	/*static*/ inline bool Registry::RegisterGlobalFunction()
	{
		constexpr auto& function = ReflectedFunction<Function>::Function;
		Registry::RegisterGlobalFunction(function, Scripting::VM::DynamicFunction::Make<function.GetFunction()>());
		return true;
	}

	template<auto Function>
	inline void Registry::RegisterDynamicGlobalFunction()
	{
		constexpr auto& function = ReflectedFunction<Function>::Function;
		RegisterDynamicGlobalFunction(function, Scripting::VM::DynamicFunction::Make<function.GetFunction()>());
	}

	template<auto Function>
	inline void Registry::DeregisterDynamicGlobalFunction()
	{
		constexpr auto& function = ReflectedFunction<Function>::Function;
		DeregisterDynamicGlobalFunction(function.GetGuid());
	}

	template<auto Event>
	/*static*/ inline bool Registry::RegisterGlobalEvent()
	{
		constexpr auto& event = ReflectedEvent<Event>::Event;
		if constexpr (event.IsGetterFunction)
		{
			Registry::RegisterGlobalEvent(event, Scripting::VM::DynamicFunction::Make<event.GetEvent()>());
		}
		else
		{
			Registry::RegisterGlobalEvent(event, Scripting::VM::DynamicFunction::MakeGetVariable<event.GetEvent()>());
		}
		return true;
	}

	template<auto Event>
	inline void Registry::RegisterDynamicGlobalEvent()
	{
		constexpr auto& event = ReflectedEvent<Event>::Event;
		if constexpr (event.IsGetterFunction)
		{
			RegisterDynamicGlobalEvent(event, Scripting::VM::DynamicFunction::Make<event.GetEvent()>());
		}
		else
		{
			RegisterDynamicGlobalEvent(event, Scripting::VM::DynamicFunction::MakeGetVariable<event.GetEvent()>());
		}
	}

	template<auto Event>
	inline void Registry::DeregisterDynamicGlobalEvent()
	{
		constexpr auto& event = ReflectedEvent<Event>::Event;
		DeregisterDynamicGlobalFunction(event.GetGuid());
	}
}

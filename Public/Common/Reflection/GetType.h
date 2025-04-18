#pragma once

#include <Common/Reflection/IsReflected.h>
#include <Common/TypeTraits/IsMemberFunction.h>
#include <Common/TypeTraits/IsMemberVariable.h>
#include <Common/TypeTraits/MemberOwnerType.h>

namespace ngine::Reflection
{
	template<typename Type>
	[[nodiscard]] constexpr const auto& GetType()
	{
		if constexpr (Internal::HasReflectedType<Type>)
		{
			return ReflectedType<Type>::Type;
		}
		else
		{
			static_unreachable("Type was not reflected");
		}
	}

	template<typename Type>
	[[nodiscard]] constexpr Guid GetTypeGuid()
	{
		return GetType<Type>().GetGuid();
	}

	template<auto Function>
	[[nodiscard]] constexpr const auto& GetFunction()
	{
		if constexpr (IsFunctionReflected<Function>)
		{
			return ReflectedFunction<Function>::Function;
		}
		else if constexpr (TypeTraits::IsMemberFunction<decltype(Function)>)
		{
			using OwnerType = TypeTraits::MemberOwnerType<decltype(Function)>;
			constexpr auto& reflectedOwnerType = Reflection::GetType<OwnerType>();

			return reflectedOwnerType.template FindFunction<Function>();
		}
		else
		{
			static_unreachable("Function was not reflected");
		}
	}

	template<auto Function>
	[[nodiscard]] constexpr Guid GetFunctionGuid()
	{
		return GetFunction<Function>().GetGuid();
	}

	template<auto Event>
	[[nodiscard]] constexpr const auto& GetEvent()
	{
		if constexpr (IsEventReflected<Event>)
		{
			return ReflectedEvent<Event>::Event;
		}
		else if constexpr (TypeTraits::IsMemberFunction<decltype(Event)>)
		{
			using OwnerType = TypeTraits::MemberOwnerType<decltype(Event)>;
			constexpr auto& reflectedOwnerType = Reflection::GetType<OwnerType>();

			return reflectedOwnerType.template FindEvent<Event>();
		}
		else
		{
			static_unreachable("Event was not reflected");
		}
	}

	template<auto Event>
	[[nodiscard]] constexpr Guid GetEventGuid()
	{
		return GetEvent<Event>().GetGuid();
	}

	template<auto Member>
	[[nodiscard]] constexpr const auto& GetProperty()
	{
		if constexpr (TypeTraits::IsMemberVariable<decltype(Member)>)
		{
			using OwnerType = TypeTraits::MemberOwnerType<decltype(Member)>;
			constexpr auto& reflectedOwnerType = Reflection::GetType<OwnerType>();

			return reflectedOwnerType.template FindProperty<Member>();
		}
		else if constexpr (TypeTraits::IsMemberFunction<decltype(Member)>)
		{
			using OwnerType = TypeTraits::MemberOwnerType<decltype(Member)>;
			constexpr auto& reflectedOwnerType = Reflection::GetType<OwnerType>();

			return reflectedOwnerType.template FindProperty<Member>();
		}
		else
		{
			static_unreachable("Property was not reflected");
		}
	}

	template<auto Function>
	[[nodiscard]] constexpr Guid GetPropertyGuid()
	{
		return GetProperty<Function>().m_guid;
	}
};

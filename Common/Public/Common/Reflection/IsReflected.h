#pragma once

#include <Common/TypeTraits/IsSame.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Reflection
{
	template<typename Type>
	struct ReflectedType;
	template<auto Function>
	struct ReflectedFunction;
	template<auto Event>
	struct ReflectedEvent;

	namespace Internal
	{
		template<typename Type>
		static auto checkHasReflectedType(int) -> decltype(ReflectedType<Type>::Type, uint8());
		template<typename Type>
		static uint16 checkHasReflectedType(...);
		template<typename Type>
		inline static constexpr bool HasReflectedType = sizeof(checkHasReflectedType<Type>(0)) == sizeof(uint8);

		template<typename Type>
		static constexpr bool CheckIsTypeReflected()
		{
			if constexpr (HasReflectedType<Type>)
			{
				return TypeTraits::IsSame<typename decltype(ReflectedType<Type>::Type)::OwnerType, Type>;
			}
			else
			{
				return false;
			}
		}

		template<auto Function>
		static auto checkHasReflectedFunction(int) -> decltype(ReflectedFunction<Function>::Function, uint8());
		template<auto Function>
		static uint16 checkHasReflectedFunction(...);

		template<auto Event>
		static auto checkHasReflectedEvent(int) -> decltype(ReflectedEvent<Event>::Event, uint8());
		template<auto Event>
		static uint16 checkHasReflectedEvent(...);
	}

	template<typename Type>
	inline static constexpr bool IsReflected = Internal::CheckIsTypeReflected<Type>();

	template<auto Function>
	inline static constexpr bool IsFunctionReflected = sizeof(Internal::checkHasReflectedFunction<Function>(0)) == sizeof(uint8);

	template<auto Event>
	inline static constexpr bool IsEventReflected = sizeof(Internal::checkHasReflectedEvent<Event>(0)) == sizeof(uint8);
};

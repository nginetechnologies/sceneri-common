#pragma once

#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/TypeTraits/IsPointer.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/TypeTraits/WithoutPointer.h>

namespace ngine::Serialization
{
	namespace Internal
	{
		template<typename T, typename... Args>
		static auto checkGlobalWrite(int)
			-> decltype(Serialize(TypeTraits::DeclareValue<const T&>(), TypeTraits::DeclareValue<Writer>(), TypeTraits::DeclareValue<Args&>()...), uint8());
		template<typename T, typename... Args>
		static uint16 checkGlobalWrite(...);
		template<typename T, typename... Args>
		static auto checkMemberWrite(int)
			-> decltype(bool{TypeTraits::DeclareValue<const T&>().Serialize(TypeTraits::DeclareValue<Writer>(), TypeTraits::DeclareValue<Args&>()...)}, uint8());
		template<typename T, typename... Args>
		static uint16 checkMemberWrite(...);

		template<typename Type, typename... Args>
		inline static constexpr bool HasGlobalWrite = sizeof(checkGlobalWrite<Type, Args...>(0)) == sizeof(uint8);
		template<typename Type, typename... Args>
		inline static constexpr bool HasMemberWrite = sizeof(checkMemberWrite<Type, Args...>(0)) == sizeof(uint8);
		template<typename Type, typename... Args>
		inline static constexpr bool HasAnyWrite = HasGlobalWrite<Type, Args...> || HasMemberWrite<Type, Args...> ||
		                                           (!TypeTraits::IsPointer<Type> && TypeTraits::IsPrimitive<Type> && sizeof...(Args) == 0) ||
		                                           (TypeTraits::IsPointer<Type> &&
		                                            (HasGlobalWrite<TypeTraits::WithoutPointer<Type>, Args...> ||
		                                             HasMemberWrite<TypeTraits::WithoutPointer<Type>, Args...>)) ||
		                                           (TypeTraits::IsEnum<Type> && sizeof...(Args) == 0);

		template<typename T, typename... Args>
		EnableIf<HasGlobalWrite<T, Args...> && !TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T& element, Writer writer, Args&... args);

		template<typename T, typename... Args>
		EnableIf<HasGlobalWrite<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T pElement, Writer writer, Args&... args);

		template<typename T, typename... Args>
		EnableIf<HasMemberWrite<T, Args...> && !TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T& element, Writer writer, Args&... args);

		template<typename T, typename... Args>
		EnableIf<HasMemberWrite<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T pElement, Writer writer, Args&... args);

		template<typename T>
		EnableIf<TypeTraits::IsPrimitive<T> && !TypeTraits::IsPointer<T>, bool> SerializeElement(const T& element, Writer writer);

		template<typename T>
		EnableIf<TypeTraits::IsEnum<T>, bool> SerializeElement(const T& element, Writer writer);

		template<typename T, typename... Args>
		static auto checkCanWrite(int)
			-> decltype(bool{SerializeElement(TypeTraits::DeclareValue<const T&>(), TypeTraits::DeclareValue<Writer>(), TypeTraits::DeclareValue<Args&>()...)}, uint8());
		template<typename T, typename... Args>
		static uint16 checkCanWrite(...);

		template<typename Type, typename... Args>
		inline static constexpr bool CanWrite = sizeof(checkCanWrite<Type, Args...>(0)) == sizeof(uint8) || HasAnyWrite<Type, Args...>;
	}
}

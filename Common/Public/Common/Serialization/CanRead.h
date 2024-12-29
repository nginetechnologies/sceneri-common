#pragma once

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/TypeTraits/IsPointer.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/TypeTraits/WithoutPointer.h>

namespace ngine::Serialization::Internal
{
	template<typename T, typename... Args>
	static auto checkGlobalRead(int)
		-> decltype(Serialize(TypeTraits::DeclareValue<T&>(), TypeTraits::DeclareValue<const Reader>(), TypeTraits::DeclareValue<Args&>()...), uint8());
	template<typename T, typename... Args>
	static uint16 checkGlobalRead(...);
	template<typename T, typename... Args>
	static auto checkMemberRead(int)
		-> decltype(bool{TypeTraits::DeclareValue<T&>().Serialize(TypeTraits::DeclareValue<const Reader>(), TypeTraits::DeclareValue<Args&>()...)}, uint8());
	template<typename T, typename... Args>
	static uint16 checkMemberRead(...);

	template<typename Type, typename... Args>
	inline static constexpr bool HasGlobalRead = sizeof(checkGlobalRead<Type, Args...>(0)) == sizeof(uint8);
	template<typename Type, typename... Args>
	inline static constexpr bool HasMemberRead = sizeof(checkMemberRead<Type, Args...>(0)) == sizeof(uint8);
	template<typename Type, typename... Args>
	inline static constexpr bool HasAnyRead = HasGlobalRead<Type, Args...> || HasMemberRead<Type, Args...> ||
	                                          (!TypeTraits::IsPointer<Type> && !TypeTraits::IsSame<Type, nullptr_type> &&
	                                           TypeTraits::IsPrimitive<Type> && sizeof...(Args) == 0) ||
	                                          (TypeTraits::IsPointer<Type> && (HasGlobalRead<TypeTraits::WithoutPointer<Type>, Args...> ||
	                                                                           HasMemberRead<TypeTraits::WithoutPointer<Type>, Args...>)) ||
	                                          (TypeTraits::IsEnum<Type> && sizeof...(Args) == 0);

	template<typename T, typename... Args>
	EnableIf<HasGlobalRead<T, Args...> && !TypeTraits::IsPointer<T>, bool> DeserializeElement(T& element, const Reader reader, Args&... args);

	template<typename T, typename... Args>
	EnableIf<HasGlobalRead<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
	DeserializeElement(T pElement, const Reader reader, Args&... args);

	template<typename T, typename... Args>
	EnableIf<HasMemberRead<T, Args...> && !TypeTraits::IsPointer<T>, bool> DeserializeElement(T& element, const Reader reader, Args&... args);

	template<typename T, typename... Args>
	EnableIf<HasMemberRead<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
	DeserializeElement(T pElement, const Reader reader, Args&... args);

	template<typename T>
	EnableIf<TypeTraits::IsPrimitive<T> && !TypeTraits::IsPointer<T> && !TypeTraits::IsSame<T, nullptr_type>, bool>
	DeserializeElement(T& element, const Reader reader);

	template<typename T>
	EnableIf<TypeTraits::IsEnum<T>, bool> DeserializeElement(T& element, const Reader reader);

	template<typename T, typename... Args>
	static auto checkCanRead(int)
		-> decltype(bool{DeserializeElement(TypeTraits::DeclareValue<T&>(), TypeTraits::DeclareValue<const Reader>(), TypeTraits::DeclareValue<Args&>()...)}, uint8());
	template<typename T, typename... Args>
	static uint16 checkCanRead(...);

	template<typename Type, typename... Args>
	inline static constexpr bool CanRead = sizeof(checkCanRead<Type, Args...>(0)) == sizeof(uint8) || HasAnyRead<Type, Args...>;
}

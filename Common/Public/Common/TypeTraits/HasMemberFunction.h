#pragma once

#include "Common/TypeTraits/IsDetected.h"

namespace ngine::TypeTraits
{
#if (defined(_MSC_VER) && (_MSC_VER >= 1939))
#define HasMemberFunctionNamed(CheckName, FunctionName, ReturnType___, ...) \
	template<typename ReturnType__, typename... Args> \
	struct Check##CheckName \
	{ \
		template<typename Type__> \
		using checkHasFunction = decltype(TypeTraits::DeclareValue<Type__&>().FunctionName(TypeTraits::DeclareValue<Args>()...)); \
	}; \
	template<typename... Args> \
	struct Check##CheckName<void, Args...> \
	{ \
		template<typename Type__> \
		using checkHasFunction = decltype(TypeTraits::DeclareValue<Type__&>().FunctionName(TypeTraits::DeclareValue<Args>()...)); \
	}; \
	template<typename ObjectType__> \
	static constexpr bool CheckName = \
		TypeTraits::IsDetectedExact<ReturnType___, Check##CheckName<ReturnType___, ##__VA_ARGS__>::template checkHasFunction, ObjectType__>;
#else
#define HasMemberFunctionNamed(CheckName, FunctionName, ReturnType___, ...) \
	template<typename ReturnType__, typename... Args> \
	struct Check##CheckName \
	{ \
		template<typename Type__> \
		static auto checkHasFunction(int) \
			-> decltype(ReturnType__{TypeTraits::DeclareValue<Type__&>().FunctionName(TypeTraits::DeclareValue<Args>()...)}, uint8()); \
		template<typename Type__> \
		static uint16 checkHasFunction(...); \
	}; \
	template<typename... Args> \
	struct Check##CheckName<void, Args...> \
	{ \
		template<typename Type__> \
		static auto checkHasFunction(int) \
			-> decltype(TypeTraits::DeclareValue<Type__&>().FunctionName(TypeTraits::DeclareValue<Args>()...), uint8()); \
		template<typename Type__> \
		static uint16 checkHasFunction(...); \
	}; \
	template<typename ObjectType__> \
	inline static constexpr bool CheckName = \
		sizeof(Check##CheckName<ReturnType___, ##__VA_ARGS__>::template checkHasFunction<ObjectType__>(0)) == sizeof(uint8);
#endif
#define HasMemberFunction(FunctionName, ReturnType___, ...) \
	HasMemberFunctionNamed(Has##FunctionName, FunctionName, ReturnType___, ##__VA_ARGS__)

#define HasTypeMemberFunctionNamed(CheckName, ObjectType, FunctionName, ReturnType__, ...) \
	HasMemberFunctionNamed(Has##CheckName##Internal, FunctionName, ReturnType__, ##__VA_ARGS__); \
	static constexpr bool CheckName = Has##CheckName##Internal<ObjectType>;

#define HasTypeMemberFunction(ObjectType, FunctionName, ReturnType__, ...) \
	HasTypeMemberFunctionNamed(Has##FunctionName, ObjectType, FunctionName, ReturnType__, ##__VA_ARGS__)
}

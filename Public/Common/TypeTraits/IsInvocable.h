#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename FunctionType, typename ReturnType, typename... Args>
		struct IsInvocable
		{
			template<typename ReturnType_, typename Type_, typename... _Args>
			static auto checkIsInvocable(int)
				-> decltype(ReturnType_{TypeTraits::DeclareValue<Type_&>()(TypeTraits::DeclareValue<_Args>()...)}, uint8());
			template<typename ReturnType_, typename Type__, typename... _Args>
			static uint16 checkIsInvocable(...);

			inline static constexpr bool Value = sizeof(checkIsInvocable<ReturnType, FunctionType, Args...>(0)) == sizeof(uint8);
		};

		template<typename FunctionType, typename... Args>
		struct IsInvocable<FunctionType, void, Args...>
		{
			template<typename Type_, typename... _Args>
			static auto checkIsInvocable(int) -> decltype(TypeTraits::DeclareValue<Type_&>()(TypeTraits::DeclareValue<_Args>()...), uint8());
			template<typename Type__, typename... _Args>
			static uint16 checkIsInvocable(...);

			inline static constexpr bool Value = sizeof(checkIsInvocable<FunctionType, Args...>(0)) == sizeof(uint8);
		};

		template<typename ClassType, typename ReturnType, typename... Args>
		struct IsInvocable<ReturnType (ClassType::*)(Args...), ReturnType, Args...>
		{
			using FunctionType = ReturnType (ClassType::*)(Args...);

			template<typename... _Args>
			static auto checkIsInvocable(int)
				-> decltype(ReturnType{(TypeTraits::DeclareValue<ClassType&>().*TypeTraits::DeclareValue<FunctionType>())(TypeTraits::DeclareValue<_Args>()...)}, uint8());
			template<typename... _Args>
			static uint16 checkIsInvocable(...);

			inline static constexpr bool Value = sizeof(checkIsInvocable<Args...>(0)) == sizeof(uint8);
		};

		template<typename ClassType, typename ReturnType, typename... Args>
		struct IsInvocable<ReturnType (ClassType::*)(Args...) const, ReturnType, Args...>
		{
			using FunctionType = ReturnType (ClassType::*)(Args...) const;

			template<typename... _Args>
			static auto checkIsInvocable(int)
				-> decltype(ReturnType{(TypeTraits::DeclareValue<ClassType&>().*TypeTraits::DeclareValue<FunctionType>())(TypeTraits::DeclareValue<_Args>()...)}, uint8());
			template<typename... _Args>
			static uint16 checkIsInvocable(...);

			inline static constexpr bool Value = sizeof(checkIsInvocable<Args...>(0)) == sizeof(uint8);
		};

		template<typename ClassType, typename... Args>
		struct IsInvocable<void (ClassType::*)(Args...), void, Args...>
		{
			using FunctionType = void (ClassType::*)(Args...);

			template<typename... _Args>
			static auto checkIsInvocable(int)
				-> decltype((TypeTraits::DeclareValue<ClassType&>().*TypeTraits::DeclareValue<FunctionType>())(TypeTraits::DeclareValue<_Args>()...), uint8());
			template<typename... _Args>
			static uint16 checkIsInvocable(...);

			inline static constexpr bool Value = sizeof(checkIsInvocable<Args...>(0)) == sizeof(uint8);
		};

		template<typename ClassType, typename... Args>
		struct IsInvocable<void (ClassType::*)(Args...) const, void, Args...>
		{
			using FunctionType = void (ClassType::*)(Args...) const;

			template<typename... _Args>
			static auto checkIsInvocable(int)
				-> decltype((TypeTraits::DeclareValue<ClassType&>().*TypeTraits::DeclareValue<FunctionType>())(TypeTraits::DeclareValue<_Args>()...), uint8());
			template<typename... _Args>
			static uint16 checkIsInvocable(...);

			inline static constexpr bool Value = sizeof(checkIsInvocable<Args...>(0)) == sizeof(uint8);
		};
	}

	template<typename FunctionType, typename ReturnType, typename... Args>
	inline static constexpr bool IsInvocable = Internal::IsInvocable<FunctionType, ReturnType, Args...>::Value;
}

#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_MEMBER_FUNCTION_BUILIN __has_builtin(__is_member_function_pointer)
#else
#define HAS_IS_MEMBER_FUNCTION_BUILIN 0
#endif

#if HAS_IS_MEMBER_FUNCTION_BUILIN
	template<typename Type>
	inline static constexpr bool IsMemberFunction = __is_member_function_pointer(Type);
#else
	namespace Internal
	{
		template<typename FunctionType>
		struct IsMemberFunction
		{
			inline static constexpr bool Value = false;
		};

		template<typename ClassType, typename ReturnType, typename... Arguments>
		struct IsMemberFunction<ReturnType (ClassType::*)(Arguments...)>
		{
			inline static constexpr bool Value = true;
		};

		template<typename ClassType, typename ReturnType, typename... Arguments>
		struct IsMemberFunction<ReturnType (ClassType::*)(Arguments...) const>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsMemberFunction = Internal::IsMemberFunction<Type>::Value;
#endif
#undef HAS_IS_MEMBER_FUNCTION_BUILIN
}

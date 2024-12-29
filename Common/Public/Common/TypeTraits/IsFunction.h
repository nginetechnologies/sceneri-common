#pragma once

#include "HasFunctionCallOperator.h"

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_FUNCTION_BUILIN __has_builtin(__is_function)
#else
#define HAS_IS_FUNCTION_BUILIN 0
#endif

#if HAS_IS_FUNCTION_BUILIN
	template<typename Type>
	inline static constexpr bool IsFunction = __is_function(Type) || HasFunctionCallOperator<Type>;
#else
	namespace Internal
	{
		template<typename FunctionType>
		struct IsFunction
		{
			inline static constexpr bool Value = HasFunctionCallOperator<FunctionType>;
		};

		template<typename ReturnType_, typename... Arguments>
		struct IsFunction<ReturnType_(Arguments...)>
		{
			inline static constexpr bool Value = true;
		};

		template<typename ReturnType_, typename... Arguments>
		struct IsFunction<ReturnType_ (*)(Arguments...)>
		{
			inline static constexpr bool Value = true;
		};

		template<typename ReturnType_, typename... Arguments>
		struct IsFunction<ReturnType_ (&)(Arguments...)>
		{
			inline static constexpr bool Value = true;
		};

		template<typename ClassType, typename ReturnType_, typename... Arguments>
		struct IsFunction<ReturnType_ (ClassType::*)(Arguments...)>
		{
			inline static constexpr bool Value = true;
		};

		template<typename ClassType, typename ReturnType_, typename... Arguments>
		struct IsFunction<ReturnType_ (ClassType::*)(Arguments...) const>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsFunction = Internal::IsFunction<Type>::Value;
#endif
#undef HAS_IS_FUNCTION_BUILIN
}

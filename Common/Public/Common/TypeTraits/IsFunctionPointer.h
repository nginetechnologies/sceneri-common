#pragma once

#include "IsFunction.h"
#include "IsPointer.h"

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_FUNCTION_POINTER_BUILIN __has_builtin(__is_function_pointer)
#else
#define HAS_IS_FUNCTION_POINTER_BUILIN 0
#endif

#if HAS_IS_FUNCTION_POINTER_BUILIN
	template<typename Type>
	inline static constexpr bool IsFunctionPointer = __is_function_pointer(Type);
#else
	namespace Internal
	{
		template<typename Type>
		struct IsFunctionPointer
		{
			inline static constexpr bool Value = false;
		};

		template<typename Type>
		struct IsFunctionPointer<Type*>
		{
			inline static constexpr bool Value = TypeTraits::IsFunction<Type>;
		};
	}

	template<typename Type>
	inline static constexpr bool IsFunctionPointer = Internal::IsFunctionPointer<Type>::Value;
#endif
#undef HAS_IS_FUNCTION_POINTER_BUILIN
}

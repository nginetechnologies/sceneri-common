#pragma once

#include "IsMemberFunction.h"

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_MEMBER_OBJECT_BUILIN __has_builtin(__is_member_object_pointer)
#else
#define HAS_IS_MEMBER_OBJECT_BUILIN 0
#endif

#if HAS_IS_MEMBER_OBJECT_BUILIN
	template<typename Type>
	inline static constexpr bool IsMemberVariable = __is_member_object_pointer(Type);
#else
	namespace Internal
	{
		template<typename Type>
		struct IsMemberVariable
		{
			inline static constexpr bool Value = false;
		};

		template<typename ClassType, typename Type>
		struct IsMemberVariable<Type ClassType::*>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsMemberVariable = Internal::IsMemberVariable<Type>::Value && !IsMemberFunction<Type>;
#endif
#undef HAS_IS_MEMBER_OBJECT_BUILIN
}

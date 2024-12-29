#pragma once

#include "IsMemberFunction.h"

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_MEMBER_POINTER_BUILIN __has_builtin(__is_member_pointer)
#else
#define HAS_IS_MEMBER_POINTER_BUILIN 0
#endif

#if HAS_IS_MEMBER_POINTER_BUILIN
	template<typename Type>
	inline static constexpr bool IsMemberPointer = __is_member_pointer(Type);
#else
	namespace Internal
	{
		template<typename Type>
		struct IsMemberPointer
		{
			inline static constexpr bool Value = false;
		};

		template<typename ClassType, typename Type>
		struct IsMemberPointer<Type ClassType::*>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsMemberPointer = Internal::IsMemberPointer<Type>::Value;
#endif
#undef HAS_IS_MEMBER_POINTER_BUILIN
}

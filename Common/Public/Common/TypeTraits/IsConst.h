#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_CONST_BUILIN __has_builtin(__is_const)
#else
#define HAS_IS_CONST_BUILIN 0
#endif

#if HAS_IS_CONST_BUILIN
	template<typename Type>
	inline static constexpr bool IsConst = __is_const(Type);
#else
	template<typename Type>
	inline static constexpr bool IsConst = false;

	template<typename Type>
	inline static constexpr bool IsConst<const Type> = true;
#endif
#undef HAS_IS_CONST_BUILIN
}

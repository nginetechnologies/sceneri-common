#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_SAME_BUILIN __has_builtin(__is_same)
#else
#define HAS_IS_SAME_BUILIN 0
#endif

#if HAS_IS_SAME_BUILIN
	template<typename Type1, typename Type2>
	inline static constexpr bool IsSame = __is_same(Type1, Type2);
#else
	template<typename Type1, typename Type2>
	inline static constexpr bool IsSame = false;

	template<typename Type>
	inline static constexpr bool IsSame<Type, Type> = true;
#endif
#undef HAS_IS_SAME_BUILIN
}

#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_VOLATILE_BUILIN __has_builtin(__is_volatile)
#else
#define HAS_IS_VOLATILE_BUILIN 0
#endif

#if HAS_IS_VOLATILE_BUILIN
	template<typename Type>
	inline static constexpr bool IsVolatile = __is_volatile(Type);
#else
	template<typename Type>
	inline static constexpr bool IsVolatile = false;

	template<typename Type>
	inline static constexpr bool IsVolatile<volatile Type> = true;
#endif
#undef HAS_IS_VOLATILE_BUILIN
}

#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_POINTER_BUILIN __has_builtin(__is_pointer)
#else
#define HAS_IS_POINTER_BUILIN 0
#endif

#if HAS_IS_POINTER_BUILIN
	template<typename Type>
	inline static constexpr bool IsPointer = __is_pointer(Type);
#else
	template<class>
	inline static constexpr bool IsPointer = false;

	template<class _Ty>
	inline static constexpr bool IsPointer<_Ty*> = true;
#endif
#undef HAS_IS_POINTER_BUILIN
}

#pragma once

namespace ngine::TypeTraits
{
#undef HAS_REMOVE_CONST_BUILIN
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_REMOVE_CONST_BUILIN __has_builtin(__remove_const)
#else
#define HAS_REMOVE_CONST_BUILIN 0
#endif

#if HAS_REMOVE_CONST_BUILIN
	template<class Type>
	using WithoutConst = __remove_const(Type);
#else
	namespace Internal
	{
		template<class InType>
		struct WithoutConst
		{
			using Type = InType;
		};

		template<class InType>
		struct WithoutConst<const InType>
		{
			using Type = InType;
		};
	}

	template<class Type>
	using WithoutConst = typename Internal::WithoutConst<Type>::Type;
#endif
}

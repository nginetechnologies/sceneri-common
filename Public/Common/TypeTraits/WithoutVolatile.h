#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_REMOVE_VOLATILE_BUILIN __has_builtin(__remove_volatile)
#else
#define HAS_REMOVE_VOLATILE_BUILIN 0
#endif

#if HAS_REMOVE_CONST_BUILIN
	template<class Type>
	using RemoveVolatile = __remove_volatile(Type);
#else
	namespace Internal
	{
		template<class InType>
		struct RemoveVolatile
		{
			using Type = InType;
		};

		template<class InType>
		struct RemoveVolatile<volatile InType>
		{
			using Type = InType;
		};
	}

	template<class Type>
	using RemoveVolatile = typename Internal::RemoveVolatile<Type>::Type;
#endif
#undef HAS_REMOVE_VOLATILE_BUILIN
}

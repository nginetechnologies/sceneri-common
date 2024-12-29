#pragma once

namespace ngine::TypeTraits
{
#undef HAS_REMOVE_CONST_BUILIN
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_REMOVE_CONST_BUILIN __has_builtin(__remove_pointer)
#define HAS_REMOVE_VOLATILE_BUILIN __has_builtin(__remove_volatile)
#else
#define HAS_REMOVE_CONST_BUILIN 0
#define HAS_REMOVE_VOLATILE_BUILIN 0
#endif

#if HAS_REMOVE_CONST_BUILIN && HAS_REMOVE_VOLATILE_BUILIN
	template<class Type>
	using WithoutConstOrVolatile = __remove_volatile(__remove_const(Type));
#else
	namespace Internal
	{
		template<class TypeIn>
		struct WithoutConstOrVolatile
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutConstOrVolatile<const TypeIn>
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutConstOrVolatile<volatile TypeIn>
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutConstOrVolatile<const volatile TypeIn>
		{
			using Type = TypeIn;
		};
	}

	template<class Type>
	using WithoutConstOrVolatile = typename Internal::WithoutConstOrVolatile<Type>::Type;
#endif
#undef HAS_REMOVE_CONST_BUILIN
#undef HAS_REMOVE_VOLATILE_BUILIN
}

#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_REMOVE_POINTER_BUILIN __has_builtin(__remove_pointer)
#else
#define HAS_REMOVE_POINTER_BUILIN 0
#endif

#if HAS_REMOVE_POINTER_BUILIN
	template<class Type>
	using WithoutPointer = __remove_pointer(Type);
#else
	namespace Internal
	{
		template<class TypeIn>
		struct WithoutPointer
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutPointer<TypeIn*>
		{
			using Type = TypeIn;
		};
	}

	template<class Type>
	using WithoutPointer = typename Internal::WithoutPointer<Type>::Type;
#endif
#undef HAS_REMOVE_POINTER_BUILIN
}

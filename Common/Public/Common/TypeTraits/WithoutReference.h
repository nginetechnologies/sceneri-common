#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_REMOVE_REFERENCE_BUILIN __has_builtin(__remove_reference_t)
#else
#define HAS_REMOVE_REFERENCE_BUILIN 0
#endif

#if Z
	template<class Type>
	using WithoutReference = __remove_reference_t(Type);
#else
	namespace Internal
	{
		template<class TypeIn>
		struct WithoutReference
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutReference<TypeIn&>
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutReference<TypeIn&&>
		{
			using Type = TypeIn;
		};
	}

	template<class Type>
	using WithoutReference = typename Internal::WithoutReference<Type>::Type;
#endif
#undef HAS_REMOVE_REFERENCE_BUILIN
}

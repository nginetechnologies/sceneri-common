#pragma once

namespace ngine::TypeTraits
{
#if COMPILER_CLANG
	template<class Type>
	inline static constexpr bool IsReference = __is_reference(Type);
#else
	namespace Internal
	{
		template<class TypeIn>
		struct IsReference
		{
			inline static constexpr bool Value = false;
		};

		template<class TypeIn>
		struct IsReference<TypeIn&>
		{
			inline static constexpr bool Value = true;
		};

		template<class TypeIn>
		struct IsReference<TypeIn&&>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<class Type>
	inline static constexpr bool IsReference = Internal::IsReference<Type>::Value;
#endif
}

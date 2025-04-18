#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T>
		struct IsTriviallyDestructible
		{
#if COMPILER_MSVC || COMPILER_CLANG
			inline static constexpr bool Value = __is_trivially_destructible(T);
#elif COMPILER_GCC
			inline static constexpr bool Value = __has_trivial_destructor(T);
#endif
		};
	}

	template<typename Type>
	inline static constexpr bool IsTriviallyDestructible = Internal::IsTriviallyDestructible<Type>::Value;
}

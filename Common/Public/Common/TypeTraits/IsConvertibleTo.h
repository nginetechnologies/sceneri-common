#pragma once

#include "TypeConstant.h"
#include "DeclareValue.h"
#include "Void.h"

namespace ngine::TypeTraits
{
#if COMPILER_MSVC || COMPILER_CLANG
	template<typename From, typename To>
	inline static constexpr bool IsConvertibleTo = __is_convertible_to(From, To);
#elif COMPILER_GCC
#if __has_builtin(__is_convertible)
	template<typename From, typename To>
	inline static constexpr bool IsConvertibleTo = __is_convertible(From, To);
#else
	namespace Internal
	{
		// Primary template: assumes the types are not convertible
		template<typename From, typename To, typename = void>
		struct IsConvertibleToImplementation : FalseType
		{
		};

		// Specialization: checks if conversion is valid using SFINAE
		template<typename From, typename To>
		struct IsConvertibleToImplementation<From, To, Void<decltype(DeclareValue<To>() = DeclareValue<From>())>> : TrueType
		{
		};

		// User-facing alias for convenience
		template<typename From, typename To>
		struct IsConvertibleTo : IsConvertibleToImplementation<From, To>
		{
		};
	}

	template<typename From, typename To>
	inline static constexpr bool IsConvertibleTo = Internal::IsConvertibleTo<From, To>::Value;
#endif
#endif
}

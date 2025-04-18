#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_INTEGRAL_BUILIN __has_builtin(__is_integral)
#else
#define HAS_IS_INTEGRAL_BUILIN 0
#endif

#if HAS_IS_INTEGRAL_BUILIN
	template<typename Type>
	inline static constexpr bool IsIntegral = __is_integral(Type);
#else
	namespace Internal
	{
		template<typename Type>
		struct IsIntegral
		{
			inline static constexpr bool Value = false;
		};

		template<>
		struct IsIntegral<bool>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<uint8>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<int8>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<uint16>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<int16>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<uint32>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<int32>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<unsigned long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<signed long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<unsigned long long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsIntegral<signed long long>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsIntegral = Internal::IsIntegral<Type>::Value;
#endif
#undef HAS_IS_INTEGRAL_BUILIN
}

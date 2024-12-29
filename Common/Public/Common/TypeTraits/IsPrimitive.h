#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_FUNDAMENTAL_BUILIN __has_builtin(__is_fundamental)
#define HAS_IS_POINTER_BUILIN __has_builtin(__is_pointer)
#define HAS_IS_REFERENCE_BUILIN __has_builtin(__is_reference)
#else
#define HAS_IS_FUNDAMENTAL_BUILIN 0
#define HAS_IS_POINTER_BUILIN 0
#define HAS_IS_REFERENCE_BUILIN 0
#endif

#if HAS_IS_FUNDAMENTAL_BUILIN && HAS_IS_POINTER_BUILIN && HAS_IS_REFERENCE_BUILIN
	template<typename Type>
	inline static constexpr bool IsPrimitive = __is_fundamental(Type) || __is_pointer(Type) || __is_reference(Type);
#else
	namespace Internal
	{
		template<typename Type>
		struct IsPrimitive
		{
			inline static constexpr bool Value = false;
		};

		template<>
		struct IsPrimitive<bool>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<uint8>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<int8>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<uint16>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<int16>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<uint32>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<int32>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<unsigned long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<signed long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<unsigned long long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<signed long long>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<float>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsPrimitive<double>
		{
			inline static constexpr bool Value = true;
		};

		template<typename Type>
		struct IsPrimitive<Type*>
		{
			inline static constexpr bool Value = true;
		};
		template<typename Type>
		struct IsPrimitive<Type&>
		{
			inline static constexpr bool Value = true;
		};

		template<>
		struct IsPrimitive<nullptr_type>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsPrimitive = Internal::IsPrimitive<Type>::Value;
#endif
#undef HAS_IS_FUNDAMENTAL_BUILIN
#undef HAS_IS_POINTER_BUILIN
#undef HAS_IS_REFERENCE_BUILIN
}

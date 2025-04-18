#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_IS_SIGNED_BUILIN __has_builtin(__is_signed)
#else
#define HAS_IS_SIGNED_BUILIN 0
#endif

#if HAS_IS_SIGNED_BUILIN
	template<typename Type>
	inline static constexpr bool IsSigned = __is_signed(Type);
	template<typename Type>
	inline static constexpr bool IsUnsigned = __is_unsigned(Type);
#else
	namespace Internal
	{
		template<typename Type>
		struct IsSigned
		{
			inline static constexpr bool Value = false;
		};

		template<>
		struct IsSigned<bool>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsSigned<int8>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsSigned<int16>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsSigned<int32>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsSigned<int64>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsSigned<float>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsSigned<double>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsSigned = Internal::IsSigned<Type>::Value;
	template<typename Type>
	inline static constexpr bool IsUnsigned = !IsSigned<Type>;
#endif
#undef HAS_IS_SIGNED_BUILIN
}

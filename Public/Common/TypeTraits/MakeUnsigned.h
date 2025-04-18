#pragma once

#include <Common/TypeTraits/Void.h>

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_MAKE_UNSIGNED_BUILIN __has_builtin(__make_unsigned)
#else
#define HAS_MAKE_UNSIGNED_BUILIN 0
#endif

#if HAS_MAKE_SIGNED_BUILIN
	template<typename Type>
	using Unsigned = __make_unsigned(Type);
#else
	namespace Internal
	{
		template<typename Type_>
		struct MakeUnsigned
		{
			using Type = Type_;
		};

		template<>
		struct MakeUnsigned<int64>
		{
			using Type = uint64;
		};
		template<>
		struct MakeUnsigned<int32>
		{
			using Type = uint32;
		};
		template<>
		struct MakeUnsigned<int16>
		{
			using Type = uint16;
		};
		template<>
		struct MakeUnsigned<int8>
		{
			using Type = uint8;
		};
	}

	template<typename Type>
	using Unsigned = typename Internal::MakeUnsigned<Type>::Type;
#endif
#undef HAS_MAKE_UNSIGNED_BUILIN
}

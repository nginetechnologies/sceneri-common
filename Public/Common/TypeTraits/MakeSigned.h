#pragma once

#include <Common/TypeTraits/Void.h>

namespace ngine::TypeTraits
{
#if COMPILER_CLANG || COMPILER_GCC
#define HAS_MAKE_SIGNED_BUILIN __has_builtin(__make_signed)
#else
#define HAS_MAKE_SIGNED_BUILIN 0
#endif

#if HAS_MAKE_SIGNED_BUILIN
	template<typename Type>
	using Signed = __make_signed(Type);
#else
	namespace Internal
	{
		template<typename Type_>
		struct MakeSigned
		{
			using Type = Type_;
		};

		template<>
		struct MakeSigned<uint64>
		{
			using Type = int64;
		};
		template<>
		struct MakeSigned<uint32>
		{
			using Type = int32;
		};
		template<>
		struct MakeSigned<uint16>
		{
			using Type = int16;
		};
		template<>
		struct MakeSigned<uint8>
		{
			using Type = int8;
		};
	}

	template<typename Type>
	using Signed = typename Internal::MakeSigned<Type>::Type;
#endif
#undef HAS_MAKE_SIGNED_BUILIN
}

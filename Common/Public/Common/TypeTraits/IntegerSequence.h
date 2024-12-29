#pragma once

#include <Common/Platform/TrivialABI.h>

namespace ngine::TypeTraits
{
	template<class T, T... I>
	struct TRIVIAL_ABI IntegerSequence
	{
		inline static constexpr T Size = sizeof...(I);
	};

#if COMPILER_CLANG || COMPILER_MSVC
	template<typename T, T N>
	using MakeIntegerSequence = __make_integer_seq<IntegerSequence, T, N>;
#elif COMPILER_GCC
	template<typename T, T N>
	using MakeIntegerSequence = IntegerSequence<T, __integer_pack(N)...>;
#endif
}

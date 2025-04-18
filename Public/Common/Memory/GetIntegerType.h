#pragma once

#include <Common/TypeTraits/Select.h>
#include <Common/Platform/CompilerWarnings.h>

namespace ngine::Memory
{
	namespace Internal
	{
		PUSH_MSVC_WARNINGS
		// Disable ''<=': expression is always true' warning. Duh, it's a constexpr template sometimes it will be.
		DISABLE_MSVC_WARNINGS(4296)

		template<size BitCount, bool Sign>
		struct GetIntegerType
		{
			using Type = TypeTraits::Select<
				(BitCount <= 8),
				TypeTraits::Select<Sign, int8, uint8>,
				TypeTraits::Select<
					(BitCount <= 16),
					TypeTraits::Select<Sign, int16, uint16>,
					TypeTraits::Select<
						(BitCount <= 32),
						TypeTraits::Select<Sign, int32, uint32>,
						TypeTraits::Select<(BitCount <= 64), TypeTraits::Select<Sign, int64, uint64>, void>>>>;
		};

		POP_MSVC_WARNINGS
	}

	template<size BitCount, bool Sign>
	using IntegerType = typename Internal::GetIntegerType<BitCount, Sign>::Type;
	template<size BitCount>
	using SignedIntegerType = IntegerType<BitCount, true>;
	template<size BitCount>
	using UnsignedIntegerType = IntegerType<BitCount, false>;
}

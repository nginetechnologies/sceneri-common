#pragma once

#include <Common/TypeTraits/ConditionalType.h>

namespace ngine::TypeTraits
{
	template<size BitCount>
	using SmallestUnsignedIntegerType = TypeTraits::ConditionalType < (BitCount <= 8),
				uint8,
				TypeTraits::ConditionalType<
					(BitCount <= 16),
					uint16,
					TypeTraits::ConditionalType<
						(BitCount <= 32),
						uint32,
						TypeTraits::ConditionalType<(BitCount <= 64), uint64, TypeTraits::ConditionalType<(BitCount <= 128), uint128, void>>>>;

	template<size BitCount>
	using SmallestSignedIntegerType = TypeTraits::ConditionalType<
		(BitCount <= 8),
		int8,
		TypeTraits::ConditionalType<
			(BitCount <= 16),
			int16,
			TypeTraits::ConditionalType<(BitCount <= 32), int32, TypeTraits::ConditionalType<(BitCount <= 64), int64, int128>>>>;

	template<size BitCount>
	using SmallestIntegerType = SmallestUnsignedIntegerType<BitCount>;
}

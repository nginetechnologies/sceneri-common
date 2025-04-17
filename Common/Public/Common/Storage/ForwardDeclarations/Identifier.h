#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	template<typename TInternalType, uint8 TIndexBitCount, TInternalType MaximumCount = (1ULL << TIndexBitCount) - 1u>
	struct TIdentifier;
}

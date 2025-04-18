#pragma once

#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename KeyType, typename ValueType>
	struct TRIVIAL_ABI Pair
	{
		KeyType key;
		ValueType value;
	};

	template<typename KeyType, typename ValueType>
	Pair(KeyType, ValueType) -> Pair<KeyType, ValueType>;
}

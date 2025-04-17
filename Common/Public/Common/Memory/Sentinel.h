#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	enum class SentinelType : uint8
	{
		NotSupported,
		//! Sentinel value known at compile-time accessed through a constexpr variable
		Static,
		//! Sentinel value known through a function call
		Dynamic
	};

	//! Used to define a sentinel value for a type, i.e.
	//! template<>
	//! struct Sentinel<int> {
	//! 	inline static constexpr SentinelType Type = SentinelType::Static;
	//! 	inline static constexpr bool IsDynamic = true;
	//! 	inline static constexpr int Value = -1;
	//! };
	//! Allows other types such as Optional to optimize for the specified type, storing and detecting the Sentinel instead of needing extra
	//! context / memory
	template<typename T>
	struct Sentinel
	{
		inline static constexpr SentinelType Type = SentinelType::NotSupported;
	};
}

#pragma once

#include "../Range.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Memory/Containers/Format/String.h>

namespace ngine::Math
{
	template<typename T>
	inline bool Serialize(Range<T>& range, const Serialization::Reader serializer)
	{
		if (const Optional<ConstStringView> rangeString = serializer.ReadInPlace<ConstStringView>())
		{
			const uint32 dividerIndex = rangeString->GetSubstring(1, rangeString->GetSize() - 1).FindFirstOf('-') + 1;

			range = Range<T>{
				rangeString->GetSubstring(0, dividerIndex).ToIntegral<T>(),
				rangeString->GetSubstring(dividerIndex + 1, rangeString->GetSize() - dividerIndex).ToIntegral<T>()
			};
			return true;
		}
		return false;
	}

	template<typename T>
	inline bool Serialize(const Range<T>& range, Serialization::Writer serializer)
	{
		return serializer.SerializeInPlace(String().Format("{}-{}", range.GetMinimum(), range.GetMaximum()));
	}

	inline bool Serialize(Range<float>& range, const Serialization::Reader serializer)
	{
		if (const Optional<ConstStringView> rangeString = serializer.ReadInPlace<ConstStringView>())
		{
			const uint32 dividerIndex = rangeString->GetSubstring(1, rangeString->GetSize() - 1).FindFirstOf('-') + 1;

			range = Math::Range<float>::MakeStartToEnd(
				rangeString->GetSubstring(0, dividerIndex).ToFloat(),
				rangeString->GetSubstring(dividerIndex + 1, rangeString->GetSize() - dividerIndex).ToFloat()
			);
			return true;
		}
		return false;
	}
}

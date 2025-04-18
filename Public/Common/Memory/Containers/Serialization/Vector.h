#pragma once

#include "../Vector.h"

#include "ArrayView.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<typename ContainedType, typename AllocatorType, uint8 Flags, typename... Args>
	inline bool Serialize(TVector<ContainedType, AllocatorType, Flags>& vector, const Serialization::Reader serializer, Args&... args)
	{
		using VectorType = TVector<ContainedType, AllocatorType, Flags>;

		const Serialization::Value& currentElement = serializer.GetValue();
		const Serialization::Array& currentArray = Serialization::Array::GetFromReference(currentElement);

		if constexpr ((Flags & Memory::VectorFlags::AllowReallocate) != 0)
		{
			vector.Reserve(vector.GetSize() + static_cast<typename VectorType::SizeType>(currentArray.GetSize()));
		}
		else
		{
			Assert(vector.GetCapacity() >= vector.GetSize() + static_cast<typename VectorType::SizeType>(currentArray.GetSize()));
		}

		uint32 numSerializedElements = 0;

		if constexpr (sizeof...(Args) > 0)
		{
			for (const Serialization::TValue& elementValue : currentArray)
			{
				ContainedType& value = vector.EmplaceBack();
				numSerializedElements +=
					static_cast<uint32>(serializer.SerializeInternal<ContainedType, Args...>(elementValue, value, Forward<Args>(args)...));
			}
		}
		else
		{
			for (const Serialization::TValue& elementValue : currentArray)
			{
				ContainedType& value = vector.EmplaceBack();
				numSerializedElements += static_cast<uint32>(serializer.SerializeInternal(elementValue, value));
			}
		}

		return numSerializedElements == currentElement.Size();
	}

	template<typename ContainedType, typename AllocatorType, uint8 Flags, typename... Args>
	inline bool Serialize(const TVector<ContainedType, AllocatorType, Flags>& vector, Serialization::Writer serializer, Args&... args)
	{
		return vector.GetView().Serialize(serializer, args...);
	}
}

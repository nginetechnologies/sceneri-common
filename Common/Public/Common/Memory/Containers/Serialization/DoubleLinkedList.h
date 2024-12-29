#pragma once

#include "../DoubleLinkedList.h"

#include "ArrayView.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<class ContainedType, typename... Args>
	inline bool Serialize(DoubleLinkedList<ContainedType>& list, const Serialization::Reader serializer, Args&... args)
	{
		const Serialization::Value& currentElement = serializer.GetValue();
		const Serialization::Array& currentArray = Serialization::Array::GetFromReference(currentElement);

		uint32 numSerializedElements = 0;

		if constexpr (sizeof...(Args) > 0)
		{
			for (const Serialization::TValue& elementValue : currentArray)
			{
				ContainedType& value = list.EmplaceBack();
				numSerializedElements +=
					static_cast<uint32>(serializer.SerializeInternal<ContainedType, Args...>(elementValue, value, Forward<Args>(args)...));
			}
		}
		else
		{
			for (const Serialization::TValue& elementValue : currentArray)
			{
				ContainedType& value = list.EmplaceBack();
				numSerializedElements += static_cast<uint32>(serializer.SerializeInternal(elementValue, value));
			}
		}

		return numSerializedElements == currentElement.Size();
	}

	template<class ContainedType, typename... Args>
	inline bool Serialize(const DoubleLinkedList<ContainedType>& list, Serialization::Writer serializer, Args&... args)
	{
		static_assert(Serialization::Internal::CanWrite<ContainedType, Args&...>);
		if (list.IsEmpty())
		{
			return false;
		}

		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);

		currentElement.Reserve(list.GetSize(), serializer.GetDocument().GetAllocator());

		bool serializedAny = false;

		for (auto it = list.begin(), endIt = list.end(); it != endIt; ++it)
		{
			const ContainedType& value = *it;
			Serialization::Value elementValue(rapidjson::Type::kObjectType);
			if (serializer.SerializeInternal(elementValue, value, args...))
			{
				currentElement.PushBack(Move(elementValue), serializer.GetDocument().GetAllocator());
				serializedAny = true;
			}
		}

		return serializedAny;
	}
}

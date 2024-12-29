#pragma once

#include "../UnorderedSet.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<typename KeyType, typename HashType, typename EqualityType, typename... Args>
	FORCE_INLINE bool Serialize(UnorderedSet<KeyType, HashType, EqualityType>& set, const Serialization::Reader serializer, Args&... args)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		const Serialization::Array& currentObject = Serialization::Array::GetFromReference(currentElement);

		set.Reserve(set.GetSize() + currentObject.GetSize());

		uint32 serializedValues = 0;

		for (const Serialization::TValue& elementValue : currentObject)
		{
			KeyType value;
			if (serializer.SerializeInternal(elementValue, value, args...))
			{
				set.Emplace(Move(value));
				serializedValues++;
			}
		}

		return serializedValues > 0;
	}

	template<typename KeyType, typename HashType, typename EqualityType, typename... Args>
	FORCE_INLINE bool Serialize(const UnorderedSet<KeyType, HashType, EqualityType>& set, Serialization::Writer serializer, Args&... args)
	{
		if (set.IsEmpty())
		{
			return false;
		}

		Serialization::Value& __restrict currentElement = serializer.GetValue();

		if (!currentElement.IsArray())
		{
			currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		}

		uint32 numSerializedElements = 0;

		for (auto it = set.begin(), endIt = set.end(); it != endIt; ++it)
		{
			Serialization::Value elementValue;
			Serialization::Writer elementWriter(elementValue, serializer.GetData());
			if (elementWriter.SerializeInPlace(*it, args...))
			{
				currentElement.PushBack(Move(elementValue), serializer.GetDocument().GetAllocator());
				numSerializedElements++;
			}
		}

		Serialization::Array::GetFromReference(currentElement).Sort();

		return numSerializedElements > 0;
	}
}

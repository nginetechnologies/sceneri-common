#pragma once

#include "../ArrayView.h"

#include <Common/Serialization/Writer.h>
#include <Common/Serialization/Reader.h>

namespace ngine
{
	template<typename ContainedType, typename SizeType, typename IndexType, typename StoredType, uint8 Flags>
	template<typename... Args>
	inline bool
	ArrayView<ContainedType, SizeType, IndexType, StoredType, Flags>::Serialize(Serialization::Writer serializer, Args&... args) const
	{
		static_assert(Serialization::Internal::CanWrite<ContainedType, Args&...>);
		if (IsEmpty())
		{
			return false;
		}

		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);

		currentElement.Reserve(GetSize(), serializer.GetDocument().GetAllocator());

		bool serializedAny = false;

		for (const ContainedType& value : *this)
		{
			Serialization::Value elementValue(rapidjson::Type::kObjectType);
			if (serializer.SerializeInternal(elementValue, value, args...))
			{
				currentElement.PushBack(Move(elementValue), serializer.GetDocument().GetAllocator());
				serializedAny = true;
			}
		}

		return serializedAny;
	}

	template<typename ContainedType, typename SizeType, typename IndexType, typename StoredType, uint8 Flags>
	template<typename... Args>
	inline bool
	ArrayView<ContainedType, SizeType, IndexType, StoredType, Flags>::Serialize(const Serialization::Reader serializer, Args&... args)
	{
		static_assert(Serialization::Internal::CanRead<ContainedType, Args&...>);
		const Serialization::Value& currentElement = serializer.GetValue();
		const Serialization::Array& currentArray = Serialization::Array::GetFromReference(currentElement);

		ArrayView view = *this;
		const bool matchedSize = view.GetSize() == currentArray.GetSize();
		if (view.HasElements())
		{
			if constexpr (sizeof...(Args) > 0)
			{
				for (const Serialization::TValue& elementValue : currentArray)
				{
					ContainedType& value = view[0];
					view += static_cast<uint32>(serializer.SerializeInternal<ContainedType, Args...>(elementValue, value, args...));
				}
			}
			else
			{
				for (const Serialization::TValue& elementValue : currentArray)
				{
					ContainedType& value = view[0];
					view += static_cast<uint32>(serializer.SerializeInternal(elementValue, value));
				}
			}
		}

		return view.IsEmpty() && matchedSize;
	}
}

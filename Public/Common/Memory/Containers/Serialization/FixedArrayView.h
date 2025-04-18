#pragma once

#include "../FixedArrayView.h"

#include <Common/Serialization/Writer.h>
#include <Common/Serialization/Reader.h>

namespace ngine
{
	template<typename ContainedType, size Size, typename IndexType, typename SizeType, uint8 Flags>
	template<typename... Args>
	inline bool
	FixedArrayView<ContainedType, Size, IndexType, SizeType, Flags>::Serialize(Serialization::Writer serializer, Args&... args) const
	{
		static_assert(Serialization::Internal::CanWrite<ContainedType, Args&...>);
		const FixedArrayView view = *this;
		if (view.IsEmpty())
		{
			return false;
		}

		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);

		currentElement.Reserve(view.GetSize(), serializer.GetDocument().GetAllocator());

		bool serializedAny = false;

		for (const ContainedType& value : view)
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

	template<typename ContainedType, size Size, typename IndexType, typename SizeType, uint8 Flags>
	template<typename... Args>
	inline bool
	FixedArrayView<ContainedType, Size, IndexType, SizeType, Flags>::Serialize(const Serialization::Reader serializer, Args&... args)
	{
		static_assert(Serialization::Internal::CanRead<ContainedType, Args&...>);
		const Serialization::Value& __restrict currentElement = serializer.GetValue();

		FixedArrayView view = *this;
		for (const Serialization::Value *it = currentElement.Begin(), *end = currentElement.End(); it != end; ++it)
		{
			const Serialization::Value& elementValue = *it;
			ContainedType& value = view[0];
			view += static_cast<uint32>(serializer.SerializeInternal(elementValue, value, args...));
		}

		return view.IsEmpty();
	}
}

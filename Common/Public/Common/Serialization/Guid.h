#pragma once

#include "../Guid.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Memory/Containers/FlatString.h>

namespace ngine
{
	FORCE_INLINE bool Serialize(Guid& guid, const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		if (currentElement.IsString() | currentElement.IsNull())
		{
			if (currentElement.IsString())
			{
				guid = Guid::TryParse(ConstStringView(currentElement.GetString(), static_cast<uint32>(currentElement.GetStringLength())));
				return guid.IsValid();
			}
			else // if(currentElement.IsNull())
			{
				guid = Guid();
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	FORCE_INLINE bool Serialize(const Guid& guid, Serialization::Writer serializer)
	{
		if (!guid.IsValid())
		{
			return false;
		}

		const ngine::FlatString<37> flatString = guid.ToString();

		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(flatString.GetData(), flatString.GetSize(), serializer.GetDocument().GetAllocator());
		return true;
	}
}

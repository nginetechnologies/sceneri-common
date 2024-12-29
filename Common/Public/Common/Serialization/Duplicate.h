#pragma once

#include "SerializedData.h"
#include "Reader.h"
#include "Writer.h"

namespace ngine::Serialization
{
	template<typename Type, typename... ReadArgs>
	inline Optional<Type> Duplicate(const Type& source, ReadArgs&... readArgs)
	{
		Serialization::Data data(Serialization::ContextFlags::Duplication);
		{
			Serialization::Writer writer(data);
			writer.SerializeInPlace(source);
		}

		Serialization::Reader reader(data);
		return reader.ReadInPlace<Type>(readArgs...);
	}
}

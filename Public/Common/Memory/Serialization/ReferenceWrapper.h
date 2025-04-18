#pragma once

#include "../ReferenceWrapper.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<typename ContainedType, typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<ContainedType, Args&...>, bool>
	Serialize(const ReferenceWrapper<ContainedType> wrapper, Serialization::Writer serializer, Args&... args)
	{
		return serializer.SerializeInPlace(*wrapper, args...);
	}

	template<typename ContainedType, typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<ContainedType, Args&...>, bool>
	Serialize(ReferenceWrapper<ContainedType> wrapper, const Serialization::Reader serializer, Args&... args)
	{
		return serializer.SerializeInPlace(*wrapper, args...);
	}
}

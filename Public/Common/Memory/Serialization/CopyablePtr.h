#pragma once

#include "../CopyablePtr.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<typename ContainedType>
	FORCE_INLINE bool Serialize(const CopyablePtr<ContainedType>& pPtr, Serialization::Writer serializer)
	{
		if (pPtr.IsValid())
		{
			return serializer.SerializeInPlace(pPtr.GetReference());
		}
		return false;
	}

	template<typename ContainedType>
	FORCE_INLINE bool Serialize(CopyablePtr<ContainedType>& pPtr, const Serialization::Reader serializer)
	{
		if (pPtr.IsValid())
		{
			return serializer.SerializeInPlace(pPtr.GetReference());
		}
		return false;
	}
}

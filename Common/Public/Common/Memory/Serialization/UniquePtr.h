#pragma once

#include "../UniquePtr.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<typename ContainedType>
	FORCE_INLINE bool Serialize(const UniquePtr<ContainedType>& pPtr, Serialization::Writer serializer)
	{
		if constexpr (Serialization::Internal::CanWrite<ContainedType>)
		{
			return serializer.SerializeInPlace(pPtr.Get());
		}
		else
		{
			return false;
		}
	}

	template<typename ContainedType>
	FORCE_INLINE bool Serialize(UniquePtr<ContainedType>& pPtr, const Serialization::Reader serializer)
	{
		if constexpr (Serialization::Internal::CanRead<ContainedType>)
		{
			if (pPtr.IsValid())
			{
				return serializer.SerializeInPlace(*pPtr.Get());
			}
			else if constexpr (TypeTraits::IsMoveConstructible<ContainedType>)
			{
				ContainedType value;
				if (serializer.SerializeInPlace(value))
				{
					pPtr.CreateInPlace(Move(value));
					return true;
				}
				else
				{
					return false;
				}
			}
			else if constexpr (TypeTraits::IsDefaultConstructible<ContainedType>)
			{
				ContainedType& value = pPtr.CreateInPlace();
				return serializer.SerializeInPlace(value);
			}
			else
			{
				static_unreachable("Type can't be deserialized as a UniquePtr without being move or default constructible");
			}
		}
		else
		{
			return false;
		}
	}
}

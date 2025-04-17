#include "Memory/Optional.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Memory/Serialization/Optional.h>

#include <limits>

namespace ngine
{
#if !PLATFORM_WEB
	namespace Memory
	{
		PURE_STATICS float Sentinel<float>::GetValue() noexcept
		{
			return std::numeric_limits<float>::quiet_NaN();
		}
		PURE_STATICS double Sentinel<double>::GetValue() noexcept
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
	}
#endif

	bool Optional<bool>::Serialize(Serialization::Writer serializer) const
	{
		return serializer.SerializeInPlace(GetValue());
	}

	bool Optional<bool>::Serialize(const Serialization::Reader serializer)
	{
		bool newValue{IsValid() ? GetValue() : false};
		if (serializer.SerializeInPlace(newValue))
		{
			*this = newValue;
			return true;
		}
		return false;
	}

	template struct Optional<double>;
	template bool Optional<double, void>::Serialize<>(Serialization::Reader);
	template struct Optional<float>;
	template bool Optional<float, void>::Serialize<>(Serialization::Reader);

	template struct Optional<bool>;
	template struct Optional<uint8>;
	template struct Optional<uint16>;
	template struct Optional<uint32>;
	template struct Optional<uint64>;
	template struct Optional<int8>;
	template struct Optional<int16>;
	template struct Optional<int32>;
	template struct Optional<int64>;
}

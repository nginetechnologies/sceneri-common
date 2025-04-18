#include <Common/Serialization/Value.h>
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/EnumFlags.h>

namespace rapidjson
{
	template class rapidjson::GenericValue<rapidjson::UTF8<>, ngine::Serialization::Internal::RapidJsonAllocator>;

	template class rapidjson::GenericDocument<
		rapidjson::UTF8<>,
		ngine::Serialization::Internal::RapidJsonAllocator,
		ngine::Serialization::Internal::RapidJsonAllocator>;

	template class rapidjson::GenericStringBuffer<rapidjson::UTF8<>, ngine::Serialization::Internal::RapidJsonAllocator>;

	template class rapidjson::PrettyWriter<
		rapidjson::GenericStringBuffer<rapidjson::UTF8<>, ngine::Serialization::Internal::RapidJsonAllocator>,
		rapidjson::UTF8<>,
		rapidjson::UTF8<>,
		ngine::Serialization::Internal::RapidJsonAllocator>;

	template struct GenericStringRef<char>;
}

namespace ngine
{
	namespace Internal
	{
		template<typename UnderlyingType>
		bool EnumFlagsBase<UnderlyingType>::Serialize(const Serialization::Reader serializer)
		{
			return serializer.SerializeInPlace(reinterpret_cast<UnderlyingType&>(*this));
		}

		template<typename UnderlyingType>
		bool EnumFlagsBase<UnderlyingType>::Serialize(Serialization::Writer serializer) const
		{
			const UnderlyingType& value = reinterpret_cast<const UnderlyingType&>(*this);
			if (value == 0)
			{
				return false;
			}

			return serializer.SerializeInPlace(value);
		}

		template struct EnumFlagsBase<uint8>;
		template struct EnumFlagsBase<uint16>;
		template struct EnumFlagsBase<uint32>;
		template struct EnumFlagsBase<uint64>;
	}
}

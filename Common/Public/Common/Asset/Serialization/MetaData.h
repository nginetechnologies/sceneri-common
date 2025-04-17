#pragma once

#include "../MetaData.h"

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Serialization/Guid.h>

namespace ngine
{
	inline bool Serialize(Asset::MetaDataType& data, const Serialization::Reader reader)
	{
		if (reader.GetValue().GetValue().IsBool())
		{
			data = reader.GetValue().GetPrimitiveValue<bool>();
			return true;
		}
		else if (const Optional<ConstStringView> string = reader.ReadInPlace<ConstStringView>())
		{
			if (const Guid guid = ngine::Guid::TryParse(*string); guid.IsValid())
			{
				data = guid;
				return true;
			}
			else if ((*string)[0] == '#')
			{
				data = Math::Color::Parse(string->GetData(), string->GetSize());
				return true;
			}
			else
			{
				data = String(*string);
				return true;
			}
		}

		return false;
	}

	inline bool Serialize(const Asset::MetaDataType& data, Serialization::Writer writer)
	{
		return data.Visit(
			[writer](const bool value) mutable -> bool
			{
				return writer.SerializeInPlace(value);
			},
			[writer](const String& value) mutable -> bool
			{
				return writer.SerializeInPlace(value.GetView());
			},
			[writer](const ngine::Guid value) mutable -> bool
			{
				return writer.SerializeInPlace(value);
			},
			[writer](const Math::Color value) mutable -> bool
			{
				return writer.SerializeInPlace(value);
			},
			[]() -> bool
			{
				ExpectUnreachable();
			}
		);
	}
}

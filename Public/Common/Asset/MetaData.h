#pragma once

#include <Common/Guid.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Math/Color.h>
#include <Common/Memory/Variant.h>

namespace ngine::Asset
{
	using MetaDataType = Variant<bool, String, ngine::Guid, Math::Color>;
	using MetaDataStorage = UnorderedMap<ngine::Guid, MetaDataType, ngine::Guid::Hash>;
}

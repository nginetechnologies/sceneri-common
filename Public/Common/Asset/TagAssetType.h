#pragma once

#include <Engine/Asset/AssetTypeEditorExtension.h>

#include <Common/Asset/AssetFormat.h>
#include <Common/Asset/AssetTypeTag.h>
#include <Common/Reflection/Type.h>

namespace ngine
{
	struct TagAssetType
	{
		inline static constexpr ngine::Asset::Format AssetFormat = {"17c0ea72-024b-43b5-9cbe-db5fb238d683"_guid, MAKE_PATH(".tag.nasset"), {}};
	};
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<TagAssetType>
	{
		inline static constexpr auto Type = Reflection::Reflect<TagAssetType>(
			TagAssetType::AssetFormat.assetTypeGuid,
			MAKE_UNICODE_LITERAL("Tag"),
			Reflection::TypeFlags{},
			Reflection::Tags{Asset::Tags::AssetType},
			Properties{},
			Functions{},
			Events{},
			Extensions{Asset::EditorTypeExtension{
				"f34c4e06-0c7d-4351-b0fc-a95389a0e960"_guid,
				"8b5d9b1a-11f4-4be8-beab-308db3c22e5a"_asset,
				"b93b68e2-997b-45b1-9e37-a67d7b422e19"_asset,
				"a9d18d3e-09fe-458c-9816-ba51ca957b6a"_asset
			}}
		);
	};
}

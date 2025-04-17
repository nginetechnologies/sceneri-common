
#pragma once

#include <Common/Asset/AssetFormat.h>
#include <Common/Asset/AssetTypeTag.h>
#include <Common/Reflection/Type.h>

namespace ngine::Asset
{
	struct FolderAssetType
	{
		inline static constexpr Format AssetFormat = {"{8D66BB74-7638-4050-B790-C711FF8A5D8A}"_guid, {}, {}, Format::Flags::IsCollection};
	};
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<Asset::FolderAssetType>
	{
		inline static constexpr auto Type = Reflection::Reflect<Asset::FolderAssetType>(
			Asset::FolderAssetType::AssetFormat.assetTypeGuid,
			MAKE_UNICODE_LITERAL("Folder"),
			Reflection::TypeFlags{},
			Reflection::Tags{Asset::Tags::AssetType}
		);
	};
}

#pragma once

#include <Common/Asset/AssetDatabase.h>

namespace ngine
{
	struct PluginInfo;
}

namespace ngine::Asset
{
	//! Device local database containing local assets that can be used for projects
	struct LocalDatabase : public Database
	{
		inline static constexpr IO::PathView DirectoryName = MAKE_PATH("LocalAssets");

		LocalDatabase();

		//! Gets the path to where the config is stored
		[[nodiscard]] static IO::Path GetConfigFilePath();
		[[nodiscard]] static IO::Path GetDirectoryPath();

		[[nodiscard]] Optional<DatabaseEntry*> RegisterAsset(const Serialization::Data& assetData, const Asset& asset);
		[[nodiscard]] Optional<DatabaseEntry*> RegisterAsset(
			const Serialization::Data& assetData,
			const Asset& asset,
			const Guid parentAssetGuid,
			const IO::PathView parentAssetExtensions,
			const IO::PathView relativePath
		);
		[[nodiscard]] Optional<DatabaseEntry*> RegisterAsset(const IO::Path& assetFilePath);
		[[nodiscard]] bool RegisterPlugin(const PluginInfo& plugin);
		[[nodiscard]] bool RegisterPluginAssets(const PluginInfo& plugin);
		[[nodiscard]] bool Save(const EnumFlags<Serialization::SavingFlags> flags);

		[[nodiscard]] static IO::Path GetAssetPath(const Guid guid, const IO::PathView assetExtension);
		[[nodiscard]] static IO::Path
		GetAssetPath(const IO::PathView& assetCacheDirectoryPath, const Guid guid, const IO::PathView assetExtension);
	protected:
		[[nodiscard]] Optional<DatabaseEntry*>
		RegisterAssetInternal(const Serialization::Data& assetData, const Asset& asset, IO::Path&& targetPath);
	};
}

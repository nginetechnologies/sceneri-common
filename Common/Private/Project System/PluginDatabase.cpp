#include "Common/Project System/PluginDatabase.h"
#include "Common/Project System/EngineInfo.h"
#include "Common/Project System/PluginInfo.h"
#include <Common/IO/FileIterator.h>

namespace ngine
{
	void PluginDatabase::RegisterPlugin(const PluginInfo& plugin)
	{
		IO::PathView pluginDatabaseRootDirectory = m_filePath.GetParentPath();

		if (!plugin.GetConfigFilePath().IsRelativeTo(pluginDatabaseRootDirectory))
		{
			pluginDatabaseRootDirectory = IO::PathView{};
		}

		RegisterAsset(
			plugin.GetGuid(),
			Asset::DatabaseEntry{PluginAssetFormat.assetTypeGuid, {}, IO::Path(plugin.GetConfigFilePath())},
			pluginDatabaseRootDirectory
		);
		Assert(plugin.GetAssetDatabaseGuid().IsValid());
		RegisterAsset(
			plugin.GetAssetDatabaseGuid(),
			Asset::DatabaseEntry{
				Database::AssetFormat.assetTypeGuid,
				{},
				IO::Path::Combine(
					plugin.GetDirectory(),
					IO::Path::Merge(plugin.GetRelativeAssetDirectory(), Database::AssetFormat.metadataFileExtension)
				)
			},
			pluginDatabaseRootDirectory
		);
	}

	[[nodiscard]] IO::Path LocalPluginDatabase::GetFilePath()
	{
		return IO::Path::Combine(IO::Path::GetUserDataDirectory(), EngineInfo::Name.GetView(), FileName);
	}

	LocalPluginDatabase::LocalPluginDatabase()
		: PluginDatabase(GetFilePath())
	{
	}

	void EnginePluginDatabase::RegisterAllDefaultPlugins(const IO::PathView engineSourceDirectory)
	{
		IO::FileIterator::TraverseDirectoryRecursive(
			IO::Path::Combine(engineSourceDirectory, MAKE_PATH("DefaultPlugins")),
			[this](IO::Path&& pluginConfigFilePath) -> IO::FileIterator::TraversalResult
			{
				if (pluginConfigFilePath.GetLeftMostExtension() == PluginAssetFormat.metadataFileExtension)
				{
					const PluginInfo plugin(Move(pluginConfigFilePath));
					RegisterPlugin(plugin);
					return IO::FileIterator::TraversalResult::SkipDirectory;
				}

				return IO::FileIterator::TraversalResult::Continue;
			}
		);
	}
}

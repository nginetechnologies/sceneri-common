#pragma once

#include <Common/Asset/AssetDatabase.h>
#include <Common/Project System/PluginAssetFormat.h>

namespace ngine
{
	struct PluginDatabase : protected Asset::Database
	{
		PluginDatabase() = default;
		PluginDatabase(IO::Path&& databaseFilePath)
			: Database{databaseFilePath, databaseFilePath.GetParentPath()}
			, m_filePath(Move(databaseFilePath))
		{
		}

		[[nodiscard]] bool HasPlugin(const Asset::Guid guid) const
		{
			return Database::HasAsset(guid);
		}

		[[nodiscard]] IO::Path FindPlugin(const Asset::Guid guid) const
		{
			if (const Optional<const Asset::DatabaseEntry*> pAssetEntry = Database::GetAssetEntry(guid))
			{
				if (pAssetEntry->m_path.IsRelative())
				{
					return IO::Path::Combine(m_filePath.GetParentPath(), pAssetEntry->m_path);
				}

				return IO::Path(pAssetEntry->m_path);
			}
			return {};
		}

		template<typename Callback>
		void IteratePlugins(Callback&& callback) const
		{
			Database::IterateAssetsOfAssetType(
				[directory = m_filePath.GetParentPath(),
			   callback = Forward<Callback>(callback)](const Asset::Guid assetGuid, const Asset::DatabaseEntry& assetEntry)
				{
					if (assetEntry.m_path.IsRelative())
					{
						callback(assetGuid, IO::Path::Combine(directory, assetEntry.m_path));
					}
					else
					{
						callback(assetGuid, assetEntry.m_path);
					}
					return Memory::CallbackResult::Continue;
				},
				PluginAssetFormat.assetTypeGuid
			);
		}

		[[nodiscard]] uint32 GetCount() const
		{
			return Database::GetAssetCount();
		}

		[[nodiscard]] IO::PathView GetFilePath() const
		{
			return m_filePath;
		}

		void RegisterPlugin(const PluginInfo& plugin);

		[[nodiscard]] bool Save(const IO::ConstZeroTerminatedPathView databaseFile, const EnumFlags<Serialization::SavingFlags> flags) = delete;
		[[nodiscard]] bool Save(const EnumFlags<Serialization::SavingFlags> flags)
		{
			return Database::Save(m_filePath, flags);
		}

		void SetFilePath(IO::Path&& filePath)
		{
			m_filePath = Forward<IO::Path>(filePath);
		}

		using Database::GetGuid;
		using Database::SetGuid;

		using Database::Serialize;
	protected:
		IO::Path m_filePath;
	};

	//! Plug-in database containing engine default plug-ins
	struct EnginePluginDatabase : public PluginDatabase
	{
		inline static constexpr IO::PathView FileName = MAKE_PATH("AvailablePlugins.json");

		using PluginDatabase::PluginDatabase;

		void RegisterAllDefaultPlugins(const IO::PathView engineSourceDirectory);
	};

	//! Plug-in database containing all available local plug-ins the user has available
	struct LocalPluginDatabase : public PluginDatabase
	{
		inline static constexpr IO::PathView FileName = MAKE_PATH("LocalPlugins.json");

		[[nodiscard]] static IO::Path GetFilePath();
		LocalPluginDatabase();
	};
}

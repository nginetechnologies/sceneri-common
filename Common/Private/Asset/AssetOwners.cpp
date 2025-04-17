#include "Asset/AssetOwners.h"

#include <Common/IO/FileIterator.h>
#include <Common/Asset/AssetDatabase.h>
#include <Common/Asset/LocalAssetDatabase.h>
#include <Common/Project System/EngineDatabase.h>
#include <Common/Project System/PluginDatabase.h>
#include <Common/Project System/EngineAssetFormat.h>
#include <Common/Project System/ProjectAssetFormat.h>
#include <Common/Project System/PluginAssetFormat.h>
#include <Common/Reflection/Registry.h>

namespace ngine::Asset
{
	Owners::Owners(const IO::PathView metaAssetFile, Context&& context)
		: m_context(Forward<Context>(context))
	{
		ngine::Guid engineGuid;
		if (m_context.GetProject().IsValid())
		{
			engineGuid = m_context.GetProject()->GetEngineGuid();
		}
		else if (m_context.GetPlugin().IsValid())
		{
			engineGuid = m_context.GetPlugin()->GetEngineGuid();
		}

		if (!m_context.HasDatabaseConfigFilePath() && metaAssetFile.IsRelativeTo(LocalDatabase::GetDirectoryPath()))
		{
			m_context.SetDatabaseConfigFilePath(LocalDatabase::GetConfigFilePath());
		}

		auto checkFile = [this, &engineGuid](const IO::PathView filePath)
		{
			const IO::PathView fileExtension = filePath.GetRightMostExtension();
			if (fileExtension == ProjectAssetFormat.metadataFileExtension)
			{
				if (!m_context.HasProject())
				{
					ProjectInfo projectInfo{IO::Path(filePath)};
					if (Ensure(projectInfo.IsValid()))
					{
						m_context.SetProject(Move(projectInfo));
						engineGuid = m_context.GetProject().Get().GetEngineGuid();
					}
				}
			}
			else if (fileExtension == EngineAssetFormat.metadataFileExtension)
			{
				if (!m_context.HasEngine())
				{
					m_context.SetEngine(EngineInfo(IO::Path(filePath)));
				}
			}
			else if (fileExtension == PluginAssetFormat.metadataFileExtension)
			{
				if (!m_context.HasPlugin())
				{
					PluginInfo pluginInfo{IO::Path(filePath)};
					if (Ensure(pluginInfo.IsValid()))
					{
						m_context.SetPlugin(Move(pluginInfo));
						engineGuid = m_context.GetPlugin().Get().GetEngineGuid();
					}
				}
			}
			else if (fileExtension == Database::AssetFormat.metadataFileExtension)
			{
				if (!m_context.HasDatabaseConfigFilePath())
				{
					m_context.SetDatabaseConfigFilePath(IO::Path(filePath));
				}
			}
		};
		checkFile(metaAssetFile);

		IO::Path currentDirectory(metaAssetFile.GetParentPath());

		do
		{
			for (IO::FileIterator fileIterator(currentDirectory); !fileIterator.ReachedEnd(); fileIterator.Next())
			{
				switch (fileIterator.GetCurrentFileType())
				{
					case IO::FileType::File:
					{
						IO::Path filePath = fileIterator.GetCurrentFilePath();
						checkFile(filePath);
					}
					break;
					default:
						break;
				}
			}

			const bool foundAll = (m_context.GetProject().IsValid() | m_context.GetPlugin().IsValid()) & m_context.GetEngine().IsValid();
			if (foundAll)
			{
				break;
			}

			currentDirectory = IO::Path(currentDirectory.GetParentPath());
		} while (!currentDirectory.IsEmpty());

		if (!m_context.GetEngine().IsValid() && engineGuid.IsValid())
		{
			if (Optional<EngineInfo> pEngineInfo = EngineDatabase().FindEngineConfig(engineGuid))
			{
				m_context.SetEngine(Move(*pEngineInfo));
			}
		}
	}

	IO::Path Owners::GetProjectLauncherPath() const
	{
		Assert(m_context.GetEngine().IsValid());
		if (m_context.GetProject().IsValid())
		{
			return m_context.GetProject().Get().GetProjectLauncherPath(m_context.GetEngine().Get());
		}

		return m_context.GetEngine().Get().GetProjectLauncherPath();
	}

	IO::Path Owners::GetDatabasePath() const
	{
		return m_context.GetDatabasePath();
	}

	IO::Path Owners::GetDatabaseRootDirectory() const
	{
		return m_context.GetDatabaseRootDirectory();
	}

	IO::Path Owners::GetAssetDirectoryPath() const
	{
		return m_context.GetAssetDirectory();
	}

	bool Owners::IsAssetInDatabase(const Guid assetGuid) const
	{
		if (m_context.HasDatabaseConfigFilePath())
		{
			if (const Optional<const Database*> pDatabase = m_context.FindOrLoadDatabase())
			{
				return pDatabase->HasAsset(assetGuid);
			}
		}
		if (m_context.HasPlugin())
		{
			const PluginInfo& plugin = m_context.GetPlugin().Get();
			if (plugin.HasAssetDirectory())
			{
				Database database;
				if (database.Load(plugin))
				{
					return database.HasAsset(assetGuid);
				}
			}
		}
		else if (m_context.HasProject())
		{
			const ProjectInfo& project = m_context.GetProject().Get();
			Database database;
			if (database.Load(project))
			{
				return database.HasAsset(assetGuid);
			}
		}
		else if (m_context.HasEngine())
		{
			const EngineInfo& engine = m_context.GetEngine().Get();
			Database database;
			if (database.Load(engine))
			{
				return database.HasAsset(assetGuid);
			}
		}
		return false;
	}

	Database Context::ComputeFullDependencyDatabase() const
	{
		Database database;

		if (m_engine)
		{
			const EngineInfo& engine = m_engine.Get();
			database.Load(engine);
		}

		if (const Optional<const Database*> pCachedDatabase = FindOrLoadDatabase())
		{
			database.Import(*pCachedDatabase);
		}

		Vector<Guid> loadedPluginDatabases;

		using LoadPluginDatabaseFunc =
			void (*)(const Guid pluginGuid, Database& database, const EnginePluginDatabase& pluginDatabase, Vector<Guid>& loadedPluginDatabases);
		static LoadPluginDatabaseFunc loadPluginDatabase =
			[](const Guid pluginGuid, Database& database, const EnginePluginDatabase& pluginDatabase, Vector<Guid>& loadedPluginDatabases)
		{
			const IO::Path pluginPath = pluginDatabase.FindPlugin(pluginGuid);
			Assert(pluginPath.HasElements());
			if (LIKELY(pluginPath.HasElements()))
			{
				const PluginInfo plugin = PluginInfo(IO::Path(pluginPath));
				Assert(plugin.IsValid());
				if (LIKELY(plugin.IsValid()))
				{
					if (plugin.HasAssetDirectory())
					{
						database.Load(plugin);
					}

					for (const Guid pluginDependencyGuid : plugin.GetDependencies())
					{
						if (!loadedPluginDatabases.Contains(pluginDependencyGuid))
						{
							loadedPluginDatabases.EmplaceBack(pluginDependencyGuid);
							loadPluginDatabase(pluginDependencyGuid, database, pluginDatabase, loadedPluginDatabases);
						}
					}
				}
			}
		};

		EnginePluginDatabase pluginDatabase(IO::Path{});
		if (m_engine)
		{
			const IO::PathView engineDirectory = m_engine->GetDirectory();
			pluginDatabase = EnginePluginDatabase(IO::Path::Combine(engineDirectory, EnginePluginDatabase::FileName));
		}

		if (m_project)
		{
			const ProjectInfo& project = m_project.Get();
			database.Load(project);

			for (const Guid pluginGuid : project.GetPluginGuids())
			{
				loadPluginDatabase(pluginGuid, database, pluginDatabase, loadedPluginDatabases);
			}
		}

		if (m_plugin)
		{
			const PluginInfo& plugin = m_plugin.Get();
			pluginDatabase.RegisterPlugin(plugin);

			loadPluginDatabase(plugin.GetGuid(), database, pluginDatabase, loadedPluginDatabases);
		}

		// Always add the asset compiler plugin as a context
		// It is needed for certain asset compilations
		constexpr Guid assetCompilerPluginGuid = "72d83e65-b5d9-4ccd-8b8b-8227c805570e"_guid;
		if (!loadedPluginDatabases.Contains(assetCompilerPluginGuid))
		{
			loadedPluginDatabases.EmplaceBack(assetCompilerPluginGuid);
			loadPluginDatabase(assetCompilerPluginGuid, database, pluginDatabase, loadedPluginDatabases);
		}

		return database;
	}
}

#pragma once

#include <Common/Project System/ProjectInfo.h>
#include <Common/Project System/EngineInfo.h>
#include <Common/Project System/PluginInfo.h>
#include <Common/Asset/AssetDatabase.h>

namespace ngine::Asset
{
	struct Context
	{
		Context() = default;
		Context(ProjectInfo&& project, EngineInfo&& engine)
			: m_project(Forward<ProjectInfo>(project))
			, m_engine(Forward<EngineInfo>(engine))
		{
		}
		Context(PluginInfo&& plugin, EngineInfo&& engine)
			: m_plugin(Forward<PluginInfo>(plugin))
			, m_engine(Forward<EngineInfo>(engine))
		{
		}
		Context(EngineInfo&& engine)
			: m_engine(Forward<EngineInfo>(engine))
		{
		}

		[[nodiscard]] IO::Path GetAssetDirectory() const
		{
			if (m_project.IsValid())
			{
				return IO::Path::Combine(m_project->GetDirectory(), m_project->GetRelativeAssetDirectory());
			}
			if (m_plugin.IsValid())
			{
				return IO::Path::Combine(m_plugin->GetDirectory(), m_plugin->GetRelativeAssetDirectory());
			}
			if (m_databaseInfo.m_configFilePath.HasElements())
			{
				return IO::Path::Combine(
					m_databaseInfo.m_configFilePath.GetParentPath(),
					m_databaseInfo.m_configFilePath.GetFileNameWithoutExtensions()
				);
			}
			if (m_engine.IsValid())
			{
				return IO::Path::Combine(m_engine->GetDirectory(), EngineInfo::EngineAssetsPath);
			}

			return {};
		}
		[[nodiscard]] Database ComputeFullDependencyDatabase() const;

		[[nodiscard]] bool IsValid() const
		{
			return m_project.IsValid() || m_plugin.IsValid() || m_engine.IsValid() || m_databaseInfo.m_configFilePath.HasElements();
		}

		[[nodiscard]] Optional<ProjectInfo>& GetProject()
		{
			return m_project;
		}
		[[nodiscard]] const Optional<ProjectInfo>& GetProject() const
		{
			return m_project;
		}
		[[nodiscard]] bool HasProject() const
		{
			return m_project.IsValid();
		}
		void SetProject(ProjectInfo&& project)
		{
			m_project = Forward<ProjectInfo>(project);
		}
		[[nodiscard]] Optional<PluginInfo>& GetPlugin()
		{
			return m_plugin;
		}
		[[nodiscard]] const Optional<PluginInfo>& GetPlugin() const
		{
			return m_plugin;
		}
		[[nodiscard]] bool HasPlugin() const
		{
			return m_plugin.IsValid();
		}
		void SetPlugin(PluginInfo&& plugin)
		{
			m_plugin = Forward<PluginInfo>(plugin);
		}
		[[nodiscard]] Optional<EngineInfo>& GetEngine()
		{
			return m_engine;
		}
		[[nodiscard]] const Optional<EngineInfo>& GetEngine() const
		{
			return m_engine;
		}
		[[nodiscard]] bool HasEngine() const
		{
			return m_engine.IsValid();
		}
		void SetEngine(EngineInfo&& engine)
		{
			m_engine = Forward<EngineInfo>(engine);
		}
		[[nodiscard]] const IO::Path& GetDatabaseConfigFilePath() const
		{
			return m_databaseInfo.m_configFilePath;
		}
		[[nodiscard]] bool HasDatabaseConfigFilePath() const
		{
			return m_databaseInfo.m_configFilePath.HasElements();
		}
		void SetDatabaseConfigFilePath(IO::Path&& path)
		{
			m_databaseInfo.m_configFilePath = Forward<IO::Path>(path);
		}
		[[nodiscard]] bool HasLoadedCachedDatabase() const
		{
			return m_databaseInfo.m_cachedDatabase.IsValid();
		}
		[[nodiscard]] Optional<const Database*> GetCachedDatabase() const
		{
			return m_databaseInfo.m_cachedDatabase.IsValid() ? Optional<const Database*>{&m_databaseInfo.m_cachedDatabase.GetUnsafe()}
			                                                 : Optional<const Database*>{};
		}
		[[nodiscard]] Optional<Database*> FindOrLoadDatabase()
		{
			if (m_databaseInfo.m_cachedDatabase.IsValid())
			{
				return &m_databaseInfo.m_cachedDatabase.GetUnsafe();
			}

			m_databaseInfo.m_cachedDatabase = Database(m_databaseInfo.m_configFilePath, m_databaseInfo.m_configFilePath.GetParentPath());
			if (m_databaseInfo.m_cachedDatabase.IsValid())
			{
				return &m_databaseInfo.m_cachedDatabase.GetUnsafe();
			}
			return Invalid;
		}
		[[nodiscard]] Optional<const Database*> FindOrLoadDatabase() const
		{
			if (m_databaseInfo.m_cachedDatabase.IsValid())
			{
				return &m_databaseInfo.m_cachedDatabase.GetUnsafe();
			}

			m_databaseInfo.m_cachedDatabase = Database(m_databaseInfo.m_configFilePath, m_databaseInfo.m_configFilePath.GetParentPath());
			if (m_databaseInfo.m_cachedDatabase.IsValid())
			{
				return &m_databaseInfo.m_cachedDatabase.GetUnsafe();
			}
			return Invalid;
		}
		[[nodiscard]] IO::Path GetDatabasePath() const
		{
			if (HasDatabaseConfigFilePath())
			{
				return GetDatabaseConfigFilePath();
			}
			if (HasPlugin())
			{
				const PluginInfo& plugin = GetPlugin().Get();

				const IO::Path assetDirectory = IO::Path::Combine(plugin.GetDirectory(), plugin.GetRelativeAssetDirectory());
				return IO::Path::Merge(assetDirectory, Database::AssetFormat.metadataFileExtension);
			}
			else if (HasProject())
			{
				const ProjectInfo& project = GetProject().Get();

				const IO::Path assetDirectory = IO::Path::Combine(project.GetDirectory(), project.GetRelativeAssetDirectory());
				return IO::Path::Merge(assetDirectory, Database::AssetFormat.metadataFileExtension);
			}
			else if (HasEngine())
			{
				const EngineInfo& engine = GetEngine().Get();

				const IO::Path assetDirectory = IO::Path::Combine(engine.GetDirectory(), EngineInfo::EngineAssetsPath);
				return IO::Path::Merge(assetDirectory, Database::AssetFormat.metadataFileExtension);
			}
			return IO::Path();
		}
		[[nodiscard]] IO::Path GetDatabaseRootDirectory() const
		{
			if (HasDatabaseConfigFilePath())
			{
				return IO::Path{GetDatabaseConfigFilePath().GetParentPath()};
			}
			if (HasPlugin())
			{
				const PluginInfo& plugin = GetPlugin().Get();
				return IO::Path{plugin.GetDirectory()};
			}
			else if (HasProject())
			{
				const ProjectInfo& project = GetProject().Get();
				return IO::Path{project.GetDirectory()};
			}
			else if (HasEngine())
			{
				const EngineInfo& engine = GetEngine().Get();
				return IO::Path{engine.GetDirectory()};
			}
			return IO::Path();
		}
	protected:
		Optional<ProjectInfo> m_project;
		Optional<PluginInfo> m_plugin;
		Optional<EngineInfo> m_engine;

		struct DatabaseInfo
		{
			IO::Path m_configFilePath;
			mutable Optional<Database> m_cachedDatabase;
		};
		DatabaseInfo m_databaseInfo;
	};
}

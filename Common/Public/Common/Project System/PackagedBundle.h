#pragma once

#include <Common/Asset/AssetDatabase.h>
#include <Common/Project System/PluginDatabase.h>
#include <Common/Project System/EngineInfo.h>
#include <Common/Project System/ProjectInfo.h>
#include <Common/Project System/PluginInfo.h>

namespace ngine
{
	//! Bundle of databases and config files (.ngine, .nplugin etc)
	//! Used to support downloading all core files as one instead of making several requests
	struct PackagedBundle
	{
		inline static constexpr IO::PathView FileName = MAKE_PATH("PackagedBundle.json");

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;

		[[nodiscard]] const EngineInfo& GetEngineInfo() const LIFETIME_BOUND
		{
			return m_engineInfo;
		}
		[[nodiscard]] EngineInfo& GetEngineInfo() LIFETIME_BOUND
		{
			return m_engineInfo;
		}
		[[nodiscard]] const ProjectInfo& GetProjectInfo() const LIFETIME_BOUND
		{
			return m_projectInfo;
		}
		[[nodiscard]] ProjectInfo& GetProjectInfo() LIFETIME_BOUND
		{
			return m_projectInfo;
		}

		[[nodiscard]] const PluginDatabase& GetPluginDatabase() const LIFETIME_BOUND
		{
			return m_pluginDatabase;
		}
		[[nodiscard]] PluginDatabase& GetPluginDatabase() LIFETIME_BOUND
		{
			return m_pluginDatabase;
		}

		struct AssetDatabase
		{
			bool Serialize(const Serialization::Reader reader);
			bool Serialize(Serialization::Writer writer) const;

			IO::Path m_relativeDirectory;
			IO::Path m_databaseRootDirectory;
			Asset::Database m_assetDatabase;
		};

		void AddAssetDatabase(const IO::PathView relativePath, const IO::PathView databaseRootDireectory, Asset::Database&& database)
		{
			Assert(database.GetGuid().IsValid());
			m_assetDatabases.Emplace(
				database.GetGuid(),
				AssetDatabase{IO::Path{relativePath}, IO::Path{databaseRootDireectory}, Forward<Asset::Database>(database)}
			);
		}

		void AddPlugin(PluginInfo&& plugin)
		{
			m_plugins.Emplace(plugin.GetGuid(), Forward<PluginInfo>(plugin));
		}
	protected:
		EngineInfo m_engineInfo;
		ProjectInfo m_projectInfo;

		PluginDatabase m_pluginDatabase;
		UnorderedMap<Guid, AssetDatabase, Guid::Hash> m_assetDatabases;
		UnorderedMap<Guid, PluginInfo, Guid::Hash> m_plugins;
	};
}

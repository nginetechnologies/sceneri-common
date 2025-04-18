#include "Common/Project System/PackagedBundle.h"

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>
#include <Common/Reflection/Registry.h>

namespace ngine
{
	bool PackagedBundle::AssetDatabase::Serialize(const Serialization::Reader reader)
	{
		reader.Serialize("path", m_relativeDirectory);
		const IO::PathView pathPrefix;
		reader.SerializeInPlace(m_assetDatabase, pathPrefix);
		return true;
	}

	bool PackagedBundle::Serialize(const Serialization::Reader reader)
	{
		bool readAny = reader.Serialize("engine", m_engineInfo);
		readAny = reader.Serialize("project", m_projectInfo);

		const IO::PathView pathPrefix;
		readAny |= reader.Serialize("available_plugins", m_pluginDatabase, pathPrefix);

		if (const Optional<Serialization::Reader> databasesReader = reader.FindSerializer("asset_databases"))
		{
			m_assetDatabases.Reserve(m_assetDatabases.GetSize() + (uint32)databasesReader->GetMemberCount());
			for (Serialization::Member<Serialization::Reader> databaseReader : databasesReader->GetMemberView())
			{
				m_assetDatabases
					.Emplace(Guid::TryParse(databaseReader.key), databaseReader.value.ReadInPlaceWithDefaultValue<AssetDatabase>(AssetDatabase{}));
			}
		}

		if (const Optional<Serialization::Reader> pluginsReader = reader.FindSerializer("plugins"))
		{
			m_plugins.Reserve(m_assetDatabases.GetSize() + (uint32)pluginsReader->GetMemberCount());
			for (Serialization::Member<Serialization::Reader> pluginReader : pluginsReader->GetMemberView())
			{
				m_plugins.Emplace(Guid::TryParse(pluginReader.key), pluginReader.value.ReadInPlaceWithDefaultValue<PluginInfo>(PluginInfo{}));
			}
		}

		return readAny;
	}

	bool PackagedBundle::AssetDatabase::Serialize(Serialization::Writer writer) const
	{
		writer.Serialize("path", m_relativeDirectory);
		writer.SerializeInPlace(m_assetDatabase, m_databaseRootDirectory);
		return true;
	}

	bool PackagedBundle::Serialize(Serialization::Writer writer) const
	{
		bool wroteAny = writer.Serialize("engine", m_engineInfo);
		wroteAny = writer.Serialize("project", m_projectInfo);

		const IO::PathView databaseRootDirectory = m_pluginDatabase.GetFilePath().GetParentPath();
		wroteAny |= writer.Serialize("available_plugins", m_pluginDatabase, databaseRootDirectory);

		writer.Serialize("asset_databases", m_assetDatabases);
		writer.Serialize("plugins", m_plugins);

		return wroteAny;
	}
}

#include "Common/Project System/PluginInfo.h"
#include <Common/Plugin/Plugin.h>

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Serialization/Version.h>
#include <Common/Memory/Containers/Serialization/Vector.h>
#include <Common/Serialization/Guid.h>

namespace ngine
{
	PluginInfo::PluginInfo(IO::Path&& path)
		: m_configFilePath(Forward<IO::Path>(path))
	{
		[[maybe_unused]] const bool wasRead = Serialization::DeserializeFromDisk(m_configFilePath, *this);
	}

	bool PluginInfo::Save(const EnumFlags<Serialization::SavingFlags> savingFlags)
	{
		return Serialization::SerializeToDisk(m_configFilePath, *this, savingFlags);
	}

	bool PluginInfo::Serialize(const Serialization::Reader serializer)
	{
		serializer.Serialize("name", m_name);
		serializer.Serialize("description", m_description);
		serializer.Serialize("version", m_version);
		const bool readGuid = serializer.Serialize("guid", m_guid);
		serializer.Serialize("database_guid", m_databaseGuid);
		serializer.Serialize("engine", m_engineGuid);
		serializer.Serialize("asset_directory", m_relativeAssetDirectory);
		serializer.Serialize("source_directory", m_relativeSourceDirectory);
		serializer.Serialize("binary_directory", m_relativeBinaryDirectory);
		serializer.Serialize("library_directory", m_relativeLibraryDirectory);

		serializer.Serialize("thumbnail_guid", m_thumbnailAssetGuid);
		serializer.Serialize("plugin_dependencies", m_pluginDependencies);

		return readGuid & m_guid.IsValid();
	}

	bool PluginInfo::Serialize(Serialization::Writer serializer) const
	{
		serializer.Serialize("name", m_name);
		serializer.Serialize("description", m_description);
		serializer.Serialize("version", m_version);
		serializer.Serialize("guid", m_guid);
		serializer.Serialize("database_guid", m_databaseGuid);
		serializer.Serialize("engine", m_engineGuid);
		serializer.Serialize("asset_directory", m_relativeAssetDirectory);
		serializer.Serialize("source_directory", m_relativeSourceDirectory);
		serializer.Serialize("binary_directory", m_relativeBinaryDirectory);
		serializer.Serialize("library_directory", m_relativeLibraryDirectory);

		serializer.Serialize("thumbnail_guid", m_thumbnailAssetGuid);
		serializer.Serialize("plugin_dependencies", m_pluginDependencies);

		return m_guid.IsValid();
	}

	struct EntryPointStorage
	{
		UnorderedMap<Guid, Plugin::EntryPoint, Guid::Hash> m_map;
	};

	/* static */ EntryPointStorage& GetEntryPointStorage()
	{
		static EntryPointStorage storage;
		return storage;
	}

	/* static */ Plugin::EntryPoint Plugin::FindEntryPoint(const Guid guid)
	{
		EntryPointStorage& storage = GetEntryPointStorage();
		const decltype(storage.m_map)::const_iterator it = storage.m_map.Find(guid);
		if (it != storage.m_map.end())
		{
			return it->second;
		}

		return nullptr;
	}

	/* static */ bool Plugin::Register(const Guid guid, EntryPoint entryPoint)
	{
		EntryPointStorage& storage = GetEntryPointStorage();
		storage.m_map.Emplace(Guid(guid), Move(entryPoint));

		return true;
	}
}

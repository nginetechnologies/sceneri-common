#include "Common/Project System/ProjectInfo.h"

#include <Common/Project System/EngineInfo.h>

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/IO/Library.h>

#include <Common/Serialization/Version.h>
#include <Common/Serialization/Guid.h>
#include <Common/Memory/Containers/Serialization/Vector.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>

namespace ngine
{
	ProjectInfo::ProjectInfo(IO::Path&& projectConfigPath)
		: m_filePath(Move(projectConfigPath))
	{
		[[maybe_unused]] const bool wasRead = Serialization::DeserializeFromDisk(m_filePath, *this);
	}

	ProjectInfo::ProjectInfo(const Serialization::Data& serializedData, IO::Path&& projectConfigPath)
		: m_filePath(Move(projectConfigPath))
	{
		Serialization::Deserialize(serializedData, *this);
	}

	IO::Path ProjectInfo::GetLocalProjectLauncherPath() const
	{
		return IO::Path::Combine(
			GetDirectory(),
			GetRelativeBinaryDirectory(),
			MAKE_PATH(PLATFORM_NAME),
			MAKE_PATH(CONFIGURATION_NAME),
			IO::Path::Merge(MAKE_PATH("ProjectLauncher"), IO::Library::ExecutablePostfix)
		);
	}

	IO::Path ProjectInfo::GetProjectLauncherPath(const EngineInfo& engineInfo) const
	{
		IO::Path executablePath = GetLocalProjectLauncherPath();
		if (!executablePath.Exists())
		{
			return engineInfo.GetProjectLauncherPath();
		}
		return executablePath;
	}

	IO::Path ProjectInfo::GetLocalEditorPath() const
	{
		return IO::Path::Combine(
			GetDirectory(),
			GetRelativeBinaryDirectory(),
			MAKE_PATH(PLATFORM_NAME),
			MAKE_PATH(CONFIGURATION_NAME),
			IO::Path::Merge(MAKE_PATH("Editor"), IO::Library::ExecutablePostfix)
		);
	}

	IO::Path ProjectInfo::GetEditorPath(const EngineInfo& engineInfo) const
	{
		IO::Path executablePath = GetLocalEditorPath();
		if (!executablePath.Exists())
		{
			return engineInfo.GetEditorPath();
		}

		return executablePath;
	}

	IO::Path ProjectInfo::GetLocalAssetCompilerPath() const
	{
		return IO::Path::Combine(
			GetDirectory(),
			GetRelativeBinaryDirectory(),
			MAKE_PATH(PLATFORM_NAME),
#if PLATFORM_APPLE_MACOS
			MAKE_PATH("AssetCompiler.app/Contents/MacOS/AssetCompiler")
#else
			IO::Path::Merge(MAKE_PATH("AssetCompiler"), IO::Library::ExecutablePostfix)
#endif
		);
	}

	IO::Path ProjectInfo::GetAssetCompilerPath(const EngineInfo& engineInfo) const
	{
		IO::Path executablePath = GetLocalAssetCompilerPath();
		if (!executablePath.Exists())
		{
			return engineInfo.GetAssetCompilerPath();
		}

		return executablePath;
	}

	bool ProjectInfo::Serialize(const Serialization::Reader serializer)
	{
		serializer.Serialize("name", m_name);
		serializer.Serialize("description", m_description);
		serializer.Serialize("guid", m_guid);
		serializer.Serialize("database_guid", m_databaseGuid);
		serializer.Serialize("version", m_version);
		serializer.Serialize("engine", m_engineGuid);
		serializer.Serialize("plugins", m_plugins);
		serializer.Serialize("asset_directory", m_relativeAssetDirectory);
		serializer.Serialize("binary_directory", m_relativeBinaryDirectory);
		serializer.Serialize("library_directory", m_relativeLibraryDirectory);
		serializer.Serialize("thumbnail_guid", m_thumbnailAssetGuid);
		serializer.Serialize("default_scene", m_defaultSceneAssetGuid);
		serializer.Serialize("tags", m_tags);
		serializer.Serialize("settings", m_settings);

		return true;
	}

	bool ProjectInfo::Serialize(Serialization::Writer serializer) const
	{
		serializer.Serialize("name", m_name);
		serializer.Serialize("description", m_description);
		serializer.Serialize("guid", m_guid);
		serializer.Serialize("database_guid", m_databaseGuid);
		serializer.Serialize("version", m_version);
		serializer.Serialize("engine", m_engineGuid);
		serializer.Serialize("plugins", m_plugins);
		serializer.Serialize("asset_directory", m_relativeAssetDirectory);
		serializer.Serialize("binary_directory", m_relativeBinaryDirectory);
		serializer.Serialize("library_directory", m_relativeLibraryDirectory);
		serializer.Serialize("thumbnail_guid", m_thumbnailAssetGuid);
		serializer.Serialize("default_scene", m_defaultSceneAssetGuid);
		serializer.Serialize("settings", m_settings);
		serializer.Serialize("tags", m_tags);

		return true;
	}

	bool Settings::Entry::Serialize(const Serialization::Reader reader)
	{
		return reader.SerializeInPlace(static_cast<Any&>(*this));
	}

	bool Settings::Entry::Serialize(Serialization::Writer writer) const
	{
		return writer.SerializeInPlace(static_cast<const Any&>(*this));
	}

	bool Settings::Serialize(const Serialization::Reader reader)
	{
		return reader.SerializeInPlace(m_map);
	}

	bool Settings::Serialize(Serialization::Writer writer) const
	{
		return writer.SerializeInPlace(m_map);
	}

	template struct UnorderedMap<Guid, Internal::SettingsEntry, Guid::Hash>;
}

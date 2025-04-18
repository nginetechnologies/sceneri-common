#pragma once

#include <Common/Version.h>
#include <Common/Asset/Guid.h>
#include <Common/IO/Path.h>
#include <Common/IO/URI.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/Containers/InlineVector.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Project System/Settings.h>
#include <Common/Tag/TagGuid.h>

namespace ngine
{
	struct EngineInfo;

	struct ProjectInfo
	{
		ProjectInfo() = default;
		ProjectInfo(IO::Path&& projectConfigPath);
		ProjectInfo(const Serialization::Data& serializedData, IO::Path&& projectConfigPath);
		ProjectInfo(UnicodeString&& name, IO::Path&& projectConfigPath, const Asset::Guid engineGuid)
			: m_filePath(Forward<IO::Path>(projectConfigPath))
			, m_guid(Guid::Generate())
			, m_name(Forward<UnicodeString>(name))
			, m_engineGuid(engineGuid)
		{
		}
		explicit ProjectInfo(const ProjectInfo&) = default;
		ProjectInfo& operator=(const ProjectInfo&) = delete;
		ProjectInfo(ProjectInfo&& other) = default;
		ProjectInfo& operator=(ProjectInfo&&) = default;
		~ProjectInfo() = default;

		[[nodiscard]] bool IsValid() const
		{
			return m_filePath.HasElements() & m_guid.IsValid();
		}

		[[nodiscard]] bool HasPlugins() const
		{
			return m_plugins.HasElements();
		}
		[[nodiscard]] const ArrayView<const Asset::Guid, uint8> GetPluginGuids() const LIFETIME_BOUND
		{
			return m_plugins;
		}
		[[nodiscard]] UnicodeString::ConstZeroTerminatedView GetZeroTerminatedName() const LIFETIME_BOUND
		{
			return m_name;
		}
		[[nodiscard]] UnicodeString::ConstView GetName() const LIFETIME_BOUND
		{
			return m_name;
		}
		[[nodiscard]] UnicodeString::ConstZeroTerminatedView GetZeroTerminatedDescription() const LIFETIME_BOUND
		{
			return m_description;
		}
		[[nodiscard]] UnicodeString::ConstView GetDescription() const LIFETIME_BOUND
		{
			return m_description;
		}
		[[nodiscard]] Asset::Guid GetGuid() const
		{
			return m_guid;
		}
		void SetGuid(const Asset::Guid guid)
		{
			m_guid = guid;
		}
		[[nodiscard]] Asset::Guid GetAssetDatabaseGuid() const
		{
			return m_databaseGuid;
		}
		void SetAssetDatabaseGuid(const Asset::Guid databaseGuid)
		{
			m_databaseGuid = databaseGuid;
		}
		[[nodiscard]] Asset::Guid GetEngineGuid() const
		{
			return m_engineGuid;
		}
		[[nodiscard]] Version GetVersion() const
		{
			return m_version;
		}

		[[nodiscard]] const IO::PathView GetDirectory() const LIFETIME_BOUND
		{
			return m_filePath.GetParentPath();
		}
		[[nodiscard]] const IO::Path& GetConfigFilePath() const LIFETIME_BOUND
		{
			return m_filePath;
		}
		[[nodiscard]] const IO::PathView GetRelativeAssetDirectory() const LIFETIME_BOUND
		{
			return m_relativeAssetDirectory;
		}
		[[nodiscard]] const IO::PathView GetRelativeBinaryDirectory() const LIFETIME_BOUND
		{
			return m_relativeBinaryDirectory;
		}
		[[nodiscard]] const IO::PathView GetRelativeLibraryDirectory() const LIFETIME_BOUND
		{
			return m_relativeLibraryDirectory;
		}

		[[nodiscard]] IO::Path GetLocalProjectLauncherPath() const;
		[[nodiscard]] IO::Path GetProjectLauncherPath(const EngineInfo& engineInfo) const;
		[[nodiscard]] IO::Path GetLocalEditorPath() const;
		[[nodiscard]] IO::Path GetEditorPath(const EngineInfo& engineInfo) const;
		[[nodiscard]] IO::Path GetLocalAssetCompilerPath() const;
		[[nodiscard]] IO::Path GetAssetCompilerPath(const EngineInfo& engineInfo) const;

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;

		void AddPlugin(const Asset::Guid pluginGuid)
		{
			m_plugins.EmplaceBack(pluginGuid);
		}

		void SetMetadataFilePath(IO::Path&& path)
		{
			m_filePath = Forward<IO::Path>(path);
		}

		void SetName(UnicodeString&& name)
		{
			m_name = Forward<UnicodeString>(name);
		}
		void SetDescription(UnicodeString&& description)
		{
			m_description = Forward<UnicodeString>(description);
		}

		void SetEngine(const Asset::Guid guid)
		{
			m_engineGuid = guid;
		}

		[[nodiscard]] Asset::Guid GetThumbnailGuid() const
		{
			return m_thumbnailAssetGuid;
		}
		void SetThumbnail(const Asset::Guid thumbnailGuid)
		{
			m_thumbnailAssetGuid = thumbnailGuid;
		}

		[[nodiscard]] Asset::Guid GetDefaultSceneGuid() const
		{
			return m_defaultSceneAssetGuid;
		}
		void SetDefaultSceneGuid(const Asset::Guid sceneGuid)
		{
			m_defaultSceneAssetGuid = sceneGuid;
		}

		[[nodiscard]] Settings& GetSettings()
		{
			return m_settings;
		}
		[[nodiscard]] const Settings& GetSettings() const
		{
			return m_settings;
		}

		[[nodiscard]] ArrayView<const Tag::Guid> GetTags() const
		{
			return m_tags;
		}
		void SetTag(const Tag::Guid tag)
		{
			m_tags.EmplaceBackUnique(Tag::Guid(tag));
		}
	protected:
		IO::Path m_filePath;
		Asset::Guid m_guid;
		Asset::Guid m_databaseGuid;
		Version m_version = Version(1, 0, 0);
		UnicodeString m_name;
		UnicodeString m_description;
		Asset::Guid m_engineGuid;
		Vector<Asset::Guid, uint8> m_plugins;
		IO::Path m_relativeAssetDirectory = IO::Path(MAKE_PATH("Assets"));
		IO::Path m_relativeBinaryDirectory = IO::Path(MAKE_PATH("bin"));
		IO::Path m_relativeLibraryDirectory = IO::Path(MAKE_PATH("lib"));

		Asset::Guid m_thumbnailAssetGuid;
		Asset::Guid m_defaultSceneAssetGuid;

		InlineVector<Tag::Guid, 2> m_tags;

		Settings m_settings;
	};
}

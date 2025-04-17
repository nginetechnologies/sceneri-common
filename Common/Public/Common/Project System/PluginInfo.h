#pragma once

#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/Containers/String.h>
#include <Common/IO/Path.h>
#include <Common/Version.h>
#include <Common/Asset/Guid.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/SavingFlags.h>
#include <Common/ForwardDeclarations/EnumFlags.h>

namespace ngine
{
	struct Engine;

	struct PluginInfo
	{
		PluginInfo() = default;
		PluginInfo(IO::Path&& configFilePath);
		PluginInfo(UnicodeString&& name, IO::Path&& pluginConfigPath, const Asset::Guid engineGuid)
			: m_configFilePath(Forward<IO::Path>(pluginConfigPath))
			, m_name(Forward<UnicodeString>(name))
			, m_guid(Guid::Generate())
			, m_engineGuid(engineGuid)
		{
		}
		explicit PluginInfo(const PluginInfo&) = default;
		PluginInfo& operator=(const PluginInfo&) = delete;
		PluginInfo(PluginInfo&& other) noexcept = default;
		PluginInfo& operator=(PluginInfo&& other) noexcept = default;
		~PluginInfo() = default;

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
		bool Save(const EnumFlags<Serialization::SavingFlags>);

		[[nodiscard]] Asset::Guid GetGuid() const
		{
			return m_guid;
		}
		[[nodiscard]] Asset::Guid GetAssetDatabaseGuid() const
		{
			return m_databaseGuid;
		}
		void SetAssetDatabaseGuid(const Asset::Guid databaseGuid)
		{
			m_databaseGuid = databaseGuid;
		}
		[[nodiscard]] bool IsValid() const
		{
			return m_configFilePath.HasElements() & m_guid.IsValid();
		}

		[[nodiscard]] Asset::Guid GetEngineGuid() const
		{
			return m_engineGuid;
		}
		[[nodiscard]] ConstUnicodeStringView GetName() const
		{
			return m_name;
		}
		[[nodiscard]] ConstUnicodeStringView GetDescription() const
		{
			return m_description;
		}
		[[nodiscard]] ArrayView<const Asset::Guid, uint8> GetDependencies() const
		{
			return m_pluginDependencies;
		}

		[[nodiscard]] const IO::Path& GetConfigFilePath() const
		{
			return m_configFilePath;
		}
		[[nodiscard]] IO::PathView GetDirectory() const
		{
			return m_configFilePath.GetParentPath();
		}

		[[nodiscard]] bool HasAssetDirectory() const
		{
			return !m_relativeAssetDirectory.IsEmpty();
		}

		[[nodiscard]] bool HasBinaryDirectory() const
		{
			return !m_relativeBinaryDirectory.IsEmpty();
		}

		[[nodiscard]] bool HasSourceDirectory() const
		{
			return !m_relativeSourceDirectory.IsEmpty();
		}

		[[nodiscard]] IO::Path GetAssetDirectory() const
		{
			return IO::Path::Combine(GetDirectory(), m_relativeAssetDirectory);
		}
		[[nodiscard]] IO::Path GetSourceDirectory() const
		{
			return IO::Path::Combine(GetDirectory(), m_relativeSourceDirectory);
		}
		[[nodiscard]] IO::Path GetBinaryDirectory() const
		{
			return IO::Path::Combine(GetDirectory(), m_relativeBinaryDirectory);
		}
		[[nodiscard]] IO::Path GetLibraryDirectory() const
		{
			return IO::Path::Combine(GetDirectory(), m_relativeLibraryDirectory);
		}

		[[nodiscard]] IO::PathView GetRelativeAssetDirectory() const
		{
			return m_relativeAssetDirectory;
		}
		[[nodiscard]] IO::PathView GetRelativeSourceDirectory() const
		{
			return m_relativeSourceDirectory;
		}
		[[nodiscard]] IO::PathView GetRelativeBinaryDirectory() const
		{
			return m_relativeBinaryDirectory;
		}
		[[nodiscard]] IO::PathView GetRelativeLibraryDirectory() const
		{
			return m_relativeLibraryDirectory;
		}

		void SetRelativeSourceDirectory(IO::Path&& sourceDirectory)
		{
			m_relativeSourceDirectory = Forward<IO::Path>(sourceDirectory);
		}

		[[nodiscard]] Asset::Guid GetThumbnailGuid() const
		{
			return m_thumbnailAssetGuid;
		}

		void SetThumbnail(const Asset::Guid thumbnailGuid)
		{
			m_thumbnailAssetGuid = thumbnailGuid;
		}
	protected:
		IO::Path m_configFilePath;
		UnicodeString m_name;
		UnicodeString m_description;
		Version m_version = Version(1, 0, 0);
		Asset::Guid m_guid;
		Asset::Guid m_databaseGuid;
		Asset::Guid m_engineGuid;
		IO::Path m_relativeAssetDirectory = IO::Path(MAKE_PATH("Assets"));
		IO::Path m_relativeSourceDirectory;
		IO::Path m_relativeBinaryDirectory = IO::Path(MAKE_PATH("bin"));
		IO::Path m_relativeLibraryDirectory = IO::Path(MAKE_PATH("lib"));

		Asset::Guid m_thumbnailAssetGuid;

		Vector<Asset::Guid, uint8> m_pluginDependencies;
	};
}

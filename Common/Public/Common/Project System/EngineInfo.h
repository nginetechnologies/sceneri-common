#pragma once

#include <Common/Version.h>
#include <Common/Asset/Guid.h>
#include <Common/IO/Path.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine
{
	struct EngineInfo
	{
		EngineInfo(IO::Path&& engineConfigPath);
		EngineInfo(IO::Path&& engineConfigPath, const Serialization::Reader reader);
		EngineInfo() = default;
		explicit EngineInfo(const EngineInfo&) = default;
		EngineInfo& operator=(const EngineInfo&) = delete;
		EngineInfo(EngineInfo&& other) = default;
		EngineInfo& operator=(EngineInfo&&) = default;
		~EngineInfo() = default;

		inline static constexpr IO::PathView EngineAssetsPath = MAKE_PATH("EngineAssets");
		inline static constexpr ConstNativeZeroTerminatedStringView NativeName = MAKE_NATIVE_LITERAL("sceneri");
		inline static constexpr ConstZeroTerminatedUnicodeStringView Name = MAKE_UNICODE_LITERAL("sceneri");
		inline static constexpr ConstZeroTerminatedStringView AsciiName = "sceneri";

		[[nodiscard]] bool IsValid() const
		{
			return m_filePath.HasElements() & m_guid.IsValid();
		}
		[[nodiscard]] const ZeroTerminatedUnicodeStringView GetName() const
		{
			return Name;
		}
		[[nodiscard]] const ZeroTerminatedStringView GetAsciiName() const
		{
			return AsciiName;
		}
		[[nodiscard]] Version GetVersion() const
		{
			return m_version;
		}
		[[nodiscard]] Asset::Guid GetGuid() const
		{
			return m_guid;
		}
		[[nodiscard]] Asset::Guid GetDatabaseGuid() const
		{
			return m_databaseGuid;
		}

		[[nodiscard]] const IO::Path& GetConfigFilePath() const
		{
			return m_filePath;
		}
		[[nodiscard]] const IO::PathView GetDirectory() const
		{
			return m_filePath.GetParentPath();
		}
		[[nodiscard]] const IO::PathView GetRelativeSourceDirectory() const
		{
			return m_relativeSourceDirectory;
		}
		[[nodiscard]] const IO::PathView GetRelativeBinaryDirectory() const
		{
			return m_relativeBinaryDirectory;
		}
		[[nodiscard]] const IO::PathView GetRelativeLibraryDirectory() const
		{
			return m_relativeLibraryDirectory;
		}

		[[nodiscard]] IO::Path GetSourceDirectory() const
		{
			return IO::Path::Combine(GetDirectory(), GetRelativeSourceDirectory());
		}

		[[nodiscard]] IO::Path GetProjectLauncherPath() const;
		[[nodiscard]] IO::Path GetEditorPath() const;
		[[nodiscard]] IO::Path GetAssetCompilerPath() const;

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		IO::Path m_filePath;
		Version m_version = Version(1, 0, 0);
		Asset::Guid m_guid;
		Asset::Guid m_databaseGuid;
		IO::Path m_relativeSourceDirectory;
		IO::Path m_relativeBinaryDirectory;
		IO::Path m_relativeLibraryDirectory;
	};
}

#include "Common/Project System/EngineInfo.h"

#include <Common/Serialization/Deserialize.h>
#include <Common/Project System/EngineAssetFormat.h>
#include <Common/IO/Library.h>
#include <Common/IO/FileIterator.h>

#include <Common/Serialization/Version.h>
#include <Common/Serialization/Guid.h>
#include <Common/Memory/Containers/Serialization/Vector.h>

#if PLATFORM_APPLE
#include <mach-o/dyld.h>
#import <Foundation/NSString.h>
#import <Foundation/Foundation.h>
#elif PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#endif

namespace ngine
{
	EngineInfo::EngineInfo(IO::Path&& engineConfigPath)
		: m_filePath(engineConfigPath)
	{
		[[maybe_unused]] const bool wasRead = Serialization::DeserializeFromDisk(m_filePath, *this);
	}

	EngineInfo::EngineInfo(IO::Path&& engineConfigPath, const Serialization::Reader reader)
		: m_filePath(engineConfigPath)
	{
		reader.SerializeInPlace(*this);
	}

	IO::Path EngineInfo::GetEditorPath() const
	{
		return IO::Path::Combine(
			GetDirectory(),
			GetRelativeBinaryDirectory(),
			MAKE_PATH(PLATFORM_NAME),
			MAKE_PATH(CONFIGURATION_NAME),
#if PLATFORM_APPLE
			IO::Path::Merge(MAKE_PATH("Editor"), IO::Library::ApplicationPostfix),
			MAKE_PATH("Contents"),
			MAKE_PATH(PLATFORM_NAME),
#endif
			IO::Path::Merge(MAKE_PATH("Editor"), IO::Library::ExecutablePostfix)
		);
	}

	IO::Path EngineInfo::GetProjectLauncherPath() const
	{
		return IO::Path::Combine(
			GetDirectory(),
			GetRelativeBinaryDirectory(),
			MAKE_PATH(PLATFORM_NAME),
#if PLATFORM_APPLE
			IO::Path::Merge(MAKE_PATH("ProjectLauncher"), IO::Library::ApplicationPostfix),
			MAKE_PATH("Contents"),
			MAKE_PATH(PLATFORM_NAME),
#endif
			IO::Path::Merge(MAKE_PATH("ProjectLauncher"), IO::Library::ExecutablePostfix)
		);
	}

	IO::Path EngineInfo::GetAssetCompilerPath() const
	{
		return IO::Path::Combine(
			GetDirectory(),
			GetRelativeBinaryDirectory(),
			MAKE_PATH(PLATFORM_NAME),
#if PLATFORM_APPLE_MACOS
			IO::Path::Merge(MAKE_PATH("AssetCompiler"), IO::Library::ApplicationPostfix),
			MAKE_PATH("Contents"),
			MAKE_PATH(PLATFORM_NAME),
#endif
			IO::Path::Merge(MAKE_PATH("AssetCompiler"), IO::Library::ExecutablePostfix)
		);
	}

	bool EngineInfo::Serialize(const Serialization::Reader serializer)
	{
		serializer.Serialize("version", m_version);
		serializer.Serialize("guid", m_guid);
		serializer.Serialize("database_guid", m_databaseGuid);
		serializer.Serialize("source_directory", m_relativeSourceDirectory);
		serializer.Serialize("binary_directory", m_relativeBinaryDirectory);
		serializer.Serialize("library_directory", m_relativeLibraryDirectory);
		serializer.Serialize("core_directory", m_relativeCoreDirectory);

		return true;
	}

	bool EngineInfo::Serialize(Serialization::Writer serializer) const
	{
		serializer.Serialize("version", m_version);
		serializer.Serialize("guid", m_guid);
		serializer.Serialize("database_guid", m_databaseGuid);
		serializer.Serialize("source_directory", m_relativeSourceDirectory);
		serializer.Serialize("binary_directory", m_relativeBinaryDirectory);
		serializer.Serialize("library_directory", m_relativeLibraryDirectory);
		serializer.Serialize("core_directory", m_relativeCoreDirectory);

		return true;
	}

	namespace ProjectSystem
	{
		IO::Path FindEngineConfigPathFromExecutableFolder(IO::PathView executableFolderPath)
		{
#if PLATFORM_APPLE_IOS || PLATFORM_APPLE_VISIONOS
			NSString* resourcePath = [[NSBundle mainBundle] resourcePath];

			const char* resourcePathString = [resourcePath UTF8String];

			executableFolderPath = IO::PathView(resourcePathString, static_cast<IO::PathView::SizeType>(resourcePath.length));
			IO::Path directory(executableFolderPath);
			for (IO::FileIterator directoryFileIterator(directory); !directoryFileIterator.ReachedEnd(); directoryFileIterator.Next())
			{
				const IO::PathView fileExtension = directoryFileIterator.GetCurrentFileName().GetRightMostExtension();

				if (fileExtension == EngineAssetFormat.metadataFileExtension)
				{
					return directoryFileIterator.GetCurrentFilePath();
				}
			}
			return {};
#else
#if PLATFORM_APPLE_MACOS
			// First query within the bundle, then fall off to the normal (not sandboxed) path)
			NSString* resourcePath = [[NSBundle mainBundle] resourcePath];

			const char* resourcePathString = [resourcePath UTF8String];

			executableFolderPath = IO::PathView(resourcePathString, static_cast<IO::PathView::SizeType>(resourcePath.length));
			IO::Path directory(executableFolderPath);
			for (IO::FileIterator directoryFileIterator(directory); !directoryFileIterator.ReachedEnd(); directoryFileIterator.Next())
			{
				const IO::PathView fileExtension = directoryFileIterator.GetCurrentFileName().GetRightMostExtension();

				if (fileExtension == EngineAssetFormat.metadataFileExtension)
				{
					return directoryFileIterator.GetCurrentFilePath();
				}
			}
#elif PLATFORM_WEB
			return IO::Path::Combine(
				IO::Path::GetExecutableDirectory(),
				IO::Path::Merge(EngineInfo::Name.GetView(), EngineAssetFormat.metadataFileExtension)
			);
#endif

			IO::Path enginePath(executableFolderPath);
			IO::PathView enginePathView(executableFolderPath);

			while (true)
			{
				for (IO::FileIterator directoryFileIterator(enginePath); !directoryFileIterator.ReachedEnd(); directoryFileIterator.Next())
				{
					const IO::PathView fileExtension = directoryFileIterator.GetCurrentFileName().GetRightMostExtension();

					if (fileExtension == EngineAssetFormat.metadataFileExtension)
					{
						return directoryFileIterator.GetCurrentFilePath();
					}
				}

				const IO::PathView parentPath = enginePathView.GetParentPath();
				if (enginePathView == parentPath)
				{
					return {};
				}

				enginePathView = parentPath;
				enginePath.AssignFrom(enginePathView);
			}
#endif
		}

		IO::Path FindEngineDirectoryFromExecutableFolder(IO::PathView executableFolderPath)
		{
			return IO::Path{FindEngineConfigPathFromExecutableFolder(executableFolderPath).GetParentPath()};
		}
	}
}

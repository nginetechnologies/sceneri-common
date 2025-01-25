#if PLATFORM_WINDOWS
#include <Platform/Windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <Platform/UndefineWindowsMacros.h>

#pragma comment(lib, "shlwapi")
#elif PLATFORM_POSIX
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <ftw.h>
#endif

#include "IO/Path.h"
#include "IO/File.h"
#include "IO/FileIterator.h"
#include "IO/PathCharType.h"
#include "IO/URIView.h"
#include "IO/URI.h"

#include <Common/Memory/Containers/Vector.h>
#include <Common/Math/Min.h>
#include <Common/Time/Timestamp.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/IO/Format/Path.h>
#include <Common/EnumFlags.h>
#include <Common/Memory/Containers/Format/String.h>

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <filesystem>

#if PLATFORM_APPLE
#include <mach-o/dyld.h>
#import <Foundation/Foundation.h>

#if PLATFORM_APPLE_IOS || PLATFORM_APPLE_VISIONOS
#import <UIKit/UIApplication.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#elif PLATFORM_APPLE_MACOS
#import <AppKit/NSApplication.h>
#endif
#elif PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#include <shellapi.h>
#include <Common/Platform/UndefineWindowsMacros.h>
#elif PLATFORM_EMSCRIPTEN
#include <emscripten/wasmfs.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#endif

#include <algorithm>

namespace ngine::IO
{
	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	bool TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::operator==(const ViewType otherPath) const
	{
		return GetView() == otherPath;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	bool TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::operator!=(const ViewType otherPath) const
	{
		return GetView() != otherPath;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	bool TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::operator>(const ViewType otherPath) const
	{
		return GetView() > otherPath;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	bool TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::operator<(const ViewType otherPath) const
	{
		return GetView() < otherPath;
	}

	bool Path::Exists() const
	{
#if PLATFORM_WINDOWS
		return PathFileExistsW(GetZeroTerminated());
#elif PLATFORM_ANDROID || PLATFORM_EMSCRIPTEN
		File file(*this, AccessModeFlags::ReadBinary);
		return file.IsValid();
#elif PLATFORM_POSIX
		struct stat buffer;
		return (stat(GetZeroTerminated(), &buffer)) == 0;
#endif
	}

	bool Path::IsRelative() const
	{
#if PLATFORM_WINDOWS
		return PathIsRelativeW(GetZeroTerminated());
#elif PLATFORM_POSIX
		return m_path.HasElements() && m_path[0] != '/';
#endif
	}

	Path Path::GetDuplicated() const
	{
		const PathView directoryPath = GetParentPath();
		const PathView originalName = GetFileNameWithoutExtensions();
		const PathView extensions = GetAllExtensions();

		uint32 duplicationNumber = 1;

		Path newPath;
		do
		{
			newPath =
				IO::Path::Combine(directoryPath, Path::StringType().Format("{}-{}{}", originalName, duplicationNumber, extensions).GetView());
			duplicationNumber++;
		} while (newPath.Exists());

		return Move(newPath);
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::IsRelativeTo(TPathView other) const
	{
		if (other.IsEmpty())
		{
			return true;
		}

		TPathView currentPath = *this;
		do
		{
			if (currentPath == other)
			{
				return true;
			}
			currentPath = currentPath.GetParentPath();
		} while (currentPath.HasElements());

		return false;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetRelativeToParent(const TPathView parent) const
	{
		const bool isRelative = IsRelativeTo(parent);
		Assert(isRelative);

		if (isRelative)
		{
			const SizeType hasSeparatorAtNewStart = parent.HasElements() && parent.GetSize() < GetSize() &&
			                                        static_cast<SizeType>(this->operator[](parent.GetSize()) == PathSeparator);
			return {GetData() + parent.GetSize() + hasSeparatorAtNewStart, (SizeType)(GetSize() - parent.GetSize() - hasSeparatorAtNewStart)};
		}
		return {};
	}

	bool Path::IsDirectoryEmpty() const
	{
		Assert(IsDirectory());
#if PLATFORM_WINDOWS
		return PathIsDirectoryEmptyW(GetZeroTerminated());
#else
		FileIterator fileIterator(*this);
		return fileIterator.ReachedEnd();
#endif
	}

	bool Path::IsDirectory() const
	{
#if PLATFORM_WINDOWS
		return (GetFileAttributesW(GetZeroTerminated()) & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif PLATFORM_POSIX
		struct stat buffer;
		if (stat(GetZeroTerminated(), &buffer) != 0)
		{
			return false;
		}
		return S_ISDIR(buffer.st_mode);
#endif
	}

	bool Path::IsFile() const
	{
		return !IsDirectory();
	}

	bool Path::IsSymbolicLink() const
	{
#if PLATFORM_WINDOWS
		return (GetFileAttributesW(GetZeroTerminated()) & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
#elif PLATFORM_POSIX
		struct stat buffer;
		return lstat(GetZeroTerminated(), &buffer) == 0;
#endif
	}

	Path Path::GetSymbolicLinkTarget() const
	{
#if PLATFORM_WINDOWS
		HANDLE fileHandle =
			CreateFileW(GetZeroTerminated(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_REPARSE_POINT, nullptr);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			PathCharType pathBuffer[MaximumPathLength];
			const DWORD filePathLength =
				GetFinalPathNameByHandleW(fileHandle, pathBuffer, Math::Min(MAX_PATH, MaximumPathLength), VOLUME_NAME_DOS);
			::CloseHandle(fileHandle);

			return Path(Path::ViewType(Path::ConstStringViewType(pathBuffer, (Path::SizeType)filePathLength)));
		}
		else
		{
			return {};
		}
#elif PLATFORM_POSIX
		struct stat buffer;
		if (lstat(GetZeroTerminated(), &buffer) == 0)
		{
			PathCharType pathBuffer[MaximumPathLength];
			const ssize_t result = readlink(GetZeroTerminated(), pathBuffer, MaximumPathLength);
			if (result >= 0)
			{
				pathBuffer[Math::Min(result, MaximumPathLength)] = '\0';
				return Path(pathBuffer, (Path::SizeType)result);
			}
			else
			{
				return {};
			}
		}
		else
		{
			return {};
		}
#endif
	}

	bool Path::RemoveFile() const
	{
		Assert(IsFile() || !Exists());
#if PLATFORM_WINDOWS
		return DeleteFileW(GetZeroTerminated());
#elif PLATFORM_POSIX
		return remove(GetZeroTerminated()) == 0;
#endif
	}

	bool Path::RemoveDirectory() const
	{
		Assert(IsDirectory() || !Exists());
#if PLATFORM_WINDOWS
		return RemoveDirectoryW(GetZeroTerminated());
#elif PLATFORM_POSIX
		return rmdir(GetZeroTerminated()) == 0;
#endif
	}

	bool Path::EmptyDirectoryRecursively() const
	{
		Assert(IsDirectory());
		for (FileIterator fileIterator(*this); !fileIterator.ReachedEnd(); fileIterator.Next())
		{
			switch (fileIterator.GetCurrentFileType())
			{
				case FileType::Directory:
				{
					const Path directoryPath = fileIterator.GetCurrentFilePath();
					if (directoryPath.EmptyDirectoryRecursively())
					{
						directoryPath.RemoveDirectory();
					}
				}
				break;
				case FileType::File:
				{
					[[maybe_unused]] const bool wasRemoved = fileIterator.GetCurrentFilePath().RemoveFile();
				}
				break;
				case FileType::Unknown:
					break;
			}
		}
		return IsDirectoryEmpty();
	}

	bool Path::MoveDirectoryTo(const ConstZeroTerminatedPathView newPath) const
	{
		Assert(Exists());

#if PLATFORM_WINDOWS
		return MoveFileW(GetZeroTerminated(), newPath);
#elif PLATFORM_POSIX
		return rename(GetZeroTerminated(), newPath);
#endif
	}

	bool Path::CopyDirectoryTo(const ConstZeroTerminatedPathView newPath) const
	{
		Assert(Exists());
		bool result = true;
		FileIterator::TraverseDirectoryRecursive(
			*this,
			[this, newPath, &result](Path&& filePath) -> FileIterator::TraversalResult
			{
				if (filePath.IsFile())
				{
					const Path newFilePath = Path::Combine(newPath.GetView(), filePath.GetRelativeToParent(*this));
					Path(newFilePath.GetParentPath()).CreateDirectories();
					result &= filePath.CopyFileTo(newFilePath);
				}
				return FileIterator::TraversalResult::Continue;
			}
		);

		return result;
	}

	bool Path::CreateSymbolicLinkToThis(const Path& symbolicLinkPath) const
	{
#if PLATFORM_WINDOWS
		const bool success = CreateSymbolicLinkW(
			symbolicLinkPath.GetZeroTerminated(),
			GetZeroTerminated(),
			(IsDirectory() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0) | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
		);
		Assert(!success || symbolicLinkPath.GetSymbolicLinkTarget() == *this);

		if (!success)
		{
			switch (GetLastError())
			{
				// In cases where symbolic links aren't allowed (outside of developer mode for some reason), just copy the file.
				case ERROR_PRIVILEGE_NOT_HELD:
					return CopyFileTo(symbolicLinkPath);
				default:
					break;
			}
		}

		return success;
#elif PLATFORM_WEB
		return CopyFileTo(symbolicLinkPath);
#elif PLATFORM_POSIX
		return symlink(GetZeroTerminated(), symbolicLinkPath.GetZeroTerminated()) == 0;
#endif
	}

	/* static */ Path Path::GetExecutablePath()
	{
#if PLATFORM_WINDOWS
		wchar_t filePath[MaximumPathLength];
		GetModuleFileNameW(GetModuleHandleW(nullptr), filePath, static_cast<DWORD>(MaximumPathLength));

		return Path(filePath, static_cast<SizeType>(lstrlenW(filePath)));
#elif PLATFORM_APPLE
		char filePath[MaximumPathLength];
		uint32_t size = sizeof(filePath);
		_NSGetExecutablePath(filePath, &size);

		return TPath(filePath, static_cast<SizeType>(strlen(filePath)));
#elif PLATFORM_WEB
		static IO::Path executablePath = []()
		{
			em_proxying_queue* queue = emscripten_proxy_get_system_queue();
			pthread_t target = emscripten_main_runtime_thread_id();
			if (target == pthread_self())
			{
				char* str = (char*)EM_ASM_PTR({ return stringToNewUTF8(self.location.href.split('?')[0]); });
				IO::Path path = IO::Path(IO::Path::StringType(IO::Path::StringType::ConstView{str, (IO::Path::SizeType)strlen(str)}));
				free(str);
				return path;
			}
			else
			{
				IO::Path path;
				[[maybe_unused]] const bool called =
					emscripten_proxy_sync(
						queue,
						target,
						[](void* pUserData)
						{
							IO::Path& path = *reinterpret_cast<IO::Path*>(pUserData);
							char* str = (char*)EM_ASM_PTR({ return stringToNewUTF8(self.location.href.split('?')[0]); });
							path = IO::Path(IO::Path::StringType(IO::Path::StringType::ConstView{str, (IO::Path::SizeType)strlen(str)}));
							free(str);
						},
						&path
					) == 1;
				return path;
			}
		}();
		return executablePath;
#elif PLATFORM_POSIX
		char filePath[MaximumPathLength];
		const ssize_t bytes = readlink("/proc/self/exe", filePath, MaximumPathLength);
		return TPath(filePath, static_cast<SizeType>(Math::Max(bytes, 0)));
#endif
	}

	/* static */ Path Path::GetExecutableDirectory()
	{
		const IO::Path executablePath = GetExecutablePath();
#if PLATFORM_WEB
		const IO::PathView extensions = executablePath.GetAllExtensions();
		if (extensions == MAKE_PATH(".html"))
		{
			return Path(executablePath.GetParentPath());
		}
		else
		{
			// Path was probably rewritten, i.e. site.com/app.html -> site.com
			// Return the path as the directory
			return executablePath;
		}
#else
		return Path(executablePath.GetParentPath());
#endif
	}

	/* static */ Path Path::GetWorkingDirectory()
	{
#if PLATFORM_WINDOWS
		wchar_t path[MaximumPathLength];
		::GetCurrentDirectoryW(MaximumPathLength, path);

		return Path(path, static_cast<SizeType>(lstrlenW(path)));
#elif PLATFORM_POSIX
		char path[MaximumPathLength];
		if (getcwd(path, MaximumPathLength) != nullptr)
		{
			return Path(path, static_cast<SizeType>(strlen(path)));
		}
		else
		{
			return {};
		}
#endif
	}

	/* static */ bool Path::SetWorkingDirectory(const ConstZeroTerminatedPathView path)
	{
#if PLATFORM_WINDOWS
		return ::SetCurrentDirectoryW(path) != 0;
#elif PLATFORM_POSIX
		return chdir(path) == 0;
#else
#error "Not implemented"
#endif
	}

	/* static */ Path Path::GetHomeDirectory()
	{
#if PLATFORM_WINDOWS
		PWSTR userDirectoryPath;
		SHGetKnownFolderPath(FOLDERID_Profile, KF_FLAG_CREATE, NULL, &userDirectoryPath);
		return TPath(ViewType(userDirectoryPath, (uint16)wcslen(userDirectoryPath)));
#elif PLATFORM_APPLE
		NSString* homeDirectory = NSHomeDirectory();
		return TPath([homeDirectory UTF8String], (Path::SizeType)[homeDirectory length]);
#elif PLATFORM_POSIX
		int uid = getuid();
		const char* homeEnv = std::getenv("HOME");
		if (uid != 0 && homeEnv)
		{
			// We only acknowledge HOME if not root.
			return Path(homeEnv, (Path::SizeType)strlen(homeEnv));
		}
		struct passwd* pw = nullptr;
		struct passwd pwd;
		long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize < 0)
		{
			bufsize = 16384;
		}
		FixedCapacityVector<char, long> buffer(Memory::ConstructWithSize, Memory::Uninitialized, bufsize);
		int error_code = getpwuid_r(uid, &pwd, buffer.GetData(), buffer.GetSize(), &pw);
		if (error_code)
		{
			return Path();
		}
		const char* tempRes = pw->pw_dir;
		if (!tempRes)
		{
			return Path();
		}
		return Path(tempRes, (Path::SizeType)strlen(tempRes));
#endif
	}

	/* static */ void Path::InitializeDataDirectories()
	{
#if PLATFORM_EMSCRIPTEN
		backend_t persistentBackend = wasmfs_create_opfs_backend();
		{
			[[maybe_unused]] int result = wasmfs_create_directory(MAKE_PATH_LITERAL("/opfs"), 0777, persistentBackend);
			Assert(result == 0);
		}
		backend_t temporaryBackend = wasmfs_create_memory_backend();
		{
			[[maybe_unused]] int result = wasmfs_create_directory(MAKE_PATH_LITERAL("/memfs"), 0777, temporaryBackend);
			Assert(result == 0);
		}
		wasmfs_flush();
#endif
	}

	/* static */ bool Path::ClearApplicationDataDirectory()
	{
		Path applicationDataDirectory = GetApplicationDataDirectory();
		return applicationDataDirectory.EmptyDirectoryRecursively();
	}

	inline static constexpr IO::PathView AppPathName { MAKE_PATH_LITERAL("MyApp") };

	/* static */ Path Path::GetApplicationDataDirectory()
	{
		static const Path applicationDataDirectory = []()
		{
#if PLATFORM_WINDOWS
			PWSTR applicationDataPath;
			SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &applicationDataPath);
			return TPath::Combine(ViewType(applicationDataPath, (uint16)wcslen(applicationDataPath)), AppPathName);
#elif PLATFORM_APPLE_MACOS
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
			NSString* documentsDirectory = [paths objectAtIndex:0];
			return TPath::Combine(
				TPath([documentsDirectory UTF8String], (Path::SizeType)[documentsDirectory length]),
				AppPathName
			);
#elif PLATFORM_APPLE_IOS || PLATFORM_APPLE_VISIONOS
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
			NSString* documentsDirectory = [paths objectAtIndex:0];
			return TPath([documentsDirectory UTF8String], (Path::SizeType)[documentsDirectory length]);
#elif PLATFORM_ANDROID
			return Internal::GetAppDataDirectory();
#elif PLATFORM_EMSCRIPTEN
			return Path{MAKE_PATH("/opfs/data")};
#elif PLATFORM_POSIX
			return TPath::Combine(GetHomeDirectory(), MAKE_PATH("Library/Caches"), AppPathName);
#endif
		}();
		return applicationDataDirectory;
	}

	/* static */ Path Path::GetApplicationCacheDirectory()
	{
		static const Path applicationCacheDirectory = []()
		{
#if PLATFORM_WINDOWS
			PWSTR applicationDataPath;
			SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &applicationDataPath);
			return TPath::Combine(ViewType(applicationDataPath, (uint16)wcslen(applicationDataPath)), AppPathName);
#elif PLATFORM_APPLE_MACOS
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
			NSString* cacheDirectory = [paths objectAtIndex:0];
			return TPath::Combine(TPath([cacheDirectory UTF8String], (Path::SizeType)[cacheDirectory length]), AppPathName);
#elif PLATFORM_APPLE_IOS || PLATFORM_APPLE_VISIONOS
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
			NSString* cacheDirectory = [paths objectAtIndex:0];
			return TPath([cacheDirectory UTF8String], (Path::SizeType)[cacheDirectory length]);
#elif PLATFORM_ANDROID
			return Internal::GetCacheDirectory();
#elif PLATFORM_EMSCRIPTEN
			return Path{MAKE_PATH("/opfs/cache")};
#elif PLATFORM_POSIX
			return TPath::Combine(GetHomeDirectory(), MAKE_PATH("Library/Caches"), AppPathName);
#endif
		}();
		return applicationCacheDirectory;
	}

	/* static */ Path Path::GetTemporaryDirectory()
	{
#if PLATFORM_WINDOWS
		Path::StringType path;
		DWORD requiredPathLength = GetTempPathW(path.GetCapacity(), path.GetData());
		if (requiredPathLength <= path.GetCapacity())
		{
			path.Resize((PathView::SizeType)requiredPathLength, Memory::Uninitialized);
		}
		else
		{
			path.Reserve((PathView::SizeType)requiredPathLength);
			requiredPathLength = GetTempPathW(path.GetCapacity(), path.GetData());
			if (requiredPathLength <= path.GetCapacity())
			{
				path.Resize((PathView::SizeType)requiredPathLength, Memory::Uninitialized);
			}
			else
			{
				Assert(false);
			}
		}

		return TPath::Combine(path.GetView(), AppPathName);
#elif PLATFORM_APPLE
		NSString* nsTemporaryDirectory = NSTemporaryDirectory();
		PathView::SizeType pathLength = (PathView::SizeType)[nsTemporaryDirectory length];
		PathView::CharType* pPathString = [nsTemporaryDirectory UTF8String];

		pathLength -= pPathString[pathLength - 1] == PathView::PathSeparator;
		const PathView temporaryDirectory(pPathString, pathLength);
		return TPath::Combine(temporaryDirectory, AppPathName);
#elif PLATFORM_ANDROID
		return Path::Combine(Internal::GetCacheDirectory(), MAKE_PATH("Temp"));
#elif PLATFORM_EMSCRIPTEN
		return Path{MAKE_PATH("/memfs/tmp")};
#elif PLATFORM_POSIX
		const char* tempDirectory = getenv("TMPDIR");
		if (tempDirectory == 0)
			tempDirectory = "/tmp";
		return TPath::Combine(PathView(tempDirectory, (PathView::SizeType)strlen(tempDirectory)), AppPathName);
#endif
	}

	/* static */ Path Path::GetUserDataDirectory()
	{
		static const Path userDataDirectory = []()
		{
#if PLATFORM_WINDOWS
			PWSTR documentsPath;
			SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL, &documentsPath);

			return TPath::Combine(TPath::ViewType(documentsPath, (uint16)wcslen(documentsPath)), AppPathName);
#elif PLATFORM_APPLE_MACOS
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString* documentsDirectory = [paths objectAtIndex:0];
			return TPath::Combine(
				TPath([documentsDirectory UTF8String], (PathView::SizeType)[documentsDirectory length]),
				AppPathName
			);
#elif PLATFORM_APPLE_IOS || PLATFORM_APPLE_VISIONOS
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString* documentsDirectory = [paths objectAtIndex:0];
			return TPath([documentsDirectory UTF8String], (PathView::SizeType)[documentsDirectory length]);
#elif PLATFORM_ANDROID
			return Path::Combine(Internal::GetAppDataDirectory(), MAKE_PATH("User"));
#elif PLATFORM_EMSCRIPTEN
			return Path{MAKE_PATH("/opfs/user")};
#elif PLATFORM_POSIX
			return TPath::Combine(GetHomeDirectory(), AppPathName);
#endif
		}();
		return userDataDirectory;
	}

	/* static */ Path Path::GetDownloadsDirectory()
	{
		static const Path downloadsDirectory = []()
		{
#if PLATFORM_WINDOWS
			PWSTR downloadsPath;
			SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_CREATE, NULL, &downloadsPath);

			return Path(TPath::ViewType(downloadsPath, (uint16)wcslen(downloadsPath)));
#elif PLATFORM_APPLE
			NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES);
			NSString* documentsDirectory = [paths objectAtIndex:0];
			return TPath([documentsDirectory UTF8String], (PathView::SizeType)[documentsDirectory length]);
#elif PLATFORM_EMSCRIPTEN
			return Path{MAKE_PATH("/opfs/user/downloads")};
#elif PLATFORM_POSIX
			return TPath::Combine(GetHomeDirectory(), MAKE_PATH("Downloads"));
#endif
		}();
		return downloadsDirectory;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetParentPath() const
	{
		if (HasElements())
		{
			auto it = end() - 1;
			auto endIt = begin() - 1;
			for (; it != endIt && *it == PathSeparator; --it)
				;

			for (; it != endIt; --it)
			{
				if (*it == PathSeparator)
				{
					// Ignore repeated slashes
					for (; it != endIt && *it == PathSeparator; --it)
						;
					it++;

					return TPathView(begin(), static_cast<SizeType>(it - begin()));
				}
			}
		}

		return TPathView();
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	void TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::MakeRelativeTo(const ViewType other)
	{
		if (IsRelativeTo(other))
		{
			MakeRelativeToParent(other);
		}
		else
		{
			const ViewType sharedParentPath = GetView().GetSharedParentPath(other);

			uint16 backCount = 1;
			{
				ViewType parentPath = other.GetParentPath();
				while (parentPath.GetSize() > 0 && parentPath != sharedParentPath)
				{
					parentPath = parentPath.GetParentPath();
					backCount++;
				}
			}

			const ViewType relativeToShared = GetView().GetRelativeToParent(sharedParentPath);
			m_path.Reserve(sharedParentPath.GetSize() + 3 * backCount + relativeToShared.GetSize());

			StringType result;

			for (; backCount != 0; backCount--)
			{
				result += "..";
				result += PathSeparator;
			}

			result += relativeToShared.GetStringView();
			m_path = Move(result);
		}
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetSharedParentPath(const TPathView other) const
	{
		TPathView sharedPath;

		for (auto start = begin(), it = start, endIt = end(); it != endIt; ++it)
		{
			if (*it == PathSeparator)
			{
				const SizeType startIndex = static_cast<SizeType>(start - begin());
				const SizeType nameLength = static_cast<SizeType>(it - start);

				if (startIndex + nameLength >= other.GetSize())
				{
					return sharedPath;
				}

				const typename TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::ConstStringViewType name = {start, nameLength};
				const typename TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::ConstStringViewType otherName =
					other.GetSubView(startIndex, nameLength);
				if (name != otherName)
				{
					return sharedPath;
				}

				sharedPath = TPathView(begin(), startIndex + nameLength);

				start = it + 1;
			}
		}

		return sharedPath;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetFirstPath() const
	{
		for (auto it = begin(), endIt = end(); it != endIt; ++it)
		{
			if (*it == PathSeparator)
			{
				return TPathView(begin(), static_cast<SizeType>(it - begin()));
			}
		}

		return *this;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetAllExtensions() const
	{
		// Get the file name without parents
		const TPathView fileName = GetFileName();

		auto firstExtensionIt = fileName.end();
		auto it = fileName.end() - 1;
		for (const auto endIt = fileName.begin() - 1; it != endIt; --it)
		{
			if (*it == '.')
			{
				firstExtensionIt = it;
			}
		}

		return TPathView(&*firstExtensionIt, static_cast<SizeType>(fileName.end() - firstExtensionIt));
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::HasExtension() const
	{
		// Get the file name without parents
		const TPathView fileName = GetFileName();
		return fileName.Contains('.');
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetRightMostExtension() const
	{
		// Get the file name without parents
		const TPathView fileName = GetFileName();

		for (auto it = fileName.end() - 1, endIt = fileName.begin() - 1; it != endIt; --it)
		{
			if (*it == '.')
			{
				return TPathView(&*it, static_cast<SizeType>(fileName.end() - it));
			}
		}

		return TPathView();
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetLeftMostExtension() const
	{
		// Get the file name without parents
		const TPathView fileName = GetFileName();

		for (auto it = fileName.begin(), endIt = fileName.end(); it != endIt; ++it)
		{
			if (*it == '.')
			{
				auto nextExtensionIt = it + 1;
				for (; (nextExtensionIt != endIt) & (*nextExtensionIt != '.'); ++nextExtensionIt)
					;

				return TPathView(&*it, static_cast<SizeType>(&*nextExtensionIt - it));
			}
		}

		return TPathView();
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::HasExactExtensions(const TPathView queriedExtension) const
	{
		if (GetSize() < queriedExtension.GetSize())
		{
			return false;
		}

		const TPathView extensions = GetAllExtensions();
		if (extensions.GetSize() < queriedExtension.GetSize())
		{
			return false;
		}

		return extensions.EqualsCaseInsensitive(queriedExtension);
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetParentExtension() const
	{
		if (HasElements())
		{
			auto it = end() - 1;
			for (auto endIt = begin() - 1; it != endIt; --it)
			{
				if (*it == '.')
				{
					return TPathView(begin(), static_cast<SizeType>(it - begin()));
				}
			}
		}

		return *this;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::StartsWithExtensions(const TPathView queriedExtension) const
	{
		if (GetSize() < queriedExtension.GetSize())
		{
			return false;
		}

		const TPathView allExtensions = GetAllExtensions();
		if (allExtensions.GetSize() < queriedExtension.GetSize())
		{
			return false;
		}

		TPathView extensions = allExtensions;
		do
		{
			if (extensions.EqualsCaseInsensitive(queriedExtension))
			{
				return true;
			}
			extensions = extensions.GetParentExtension();
		} while (extensions.HasExtension());

		return false;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::EndsWithExtensions(const TPathView queriedExtension) const
	{
		if (GetSize() < queriedExtension.GetSize())
		{
			return false;
		}

		const TPathView allExtensions = GetAllExtensions();
		if (allExtensions.GetSize() < queriedExtension.GetSize())
		{
			return false;
		}

		TPathView extensions = allExtensions;
		do
		{
			if (extensions.EqualsCaseInsensitive(queriedExtension))
			{
				return true;
			}

			// Remove the first extension found
			Assert(extensions[0] == MAKE_PATH_LITERAL('.'));
			static auto getSecondExtension = [](const TPathView extensions) -> TPathView
			{
				for (const CharType *it = extensions.begin() + 1, *endIt = extensions.end(); it < endIt; ++it)
				{
					if (*it == '.')
					{
						return TPathView(it, static_cast<SizeType>(endIt - it));
					}
				}
				return {};
			};
			extensions = getSecondExtension(extensions);
		} while (extensions.HasExtension());

		return false;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetFileName() const
	{
		for (auto it = end() - 1, endIt = begin() - 1; it != endIt; --it)
		{
			if (*it == PathSeparator)
			{
				++it;

				TPathView fullFileName(it, static_cast<SizeType>(end() - it));
				if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
				{
					const SizeType queryStartIndex = fullFileName.FindFirstOf('?');
					if (queryStartIndex != InvalidPosition)
					{
						fullFileName = fullFileName.GetSubView(0, queryStartIndex);
					}
				}

				return fullFileName;
			}
		}

		return *this;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetFileNameWithoutExtensions() const
	{
		// Get the file name without parents
		const TPathView fileName = GetFileName();

		for (auto it = fileName.begin(), endIt = fileName.end(); it != endIt; ++it)
		{
			if (*it == '.')
			{
				return TPathView(fileName.begin(), static_cast<SizeType>(it - fileName.begin()));
			}
		}

		return fileName;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetWithoutExtensions() const
	{
		// Get the file name without parents
		const TPathView fileName = GetFileName();

		for (auto it = fileName.begin(), endIt = fileName.end(); it != endIt; ++it)
		{
			if (*it == '.')
			{
				return TPathView(begin(), static_cast<SizeType>(it - begin()));
			}
		}

		return fileName;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetWithoutQueryString() const
	{
		if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
		{
			for (auto it = end() - 1, endIt = begin() - 1; it != endIt; --it)
			{
				if (*it == '?')
				{
					return TPathView(begin(), static_cast<SizeType>(it - begin()));
				}
			}
		}

		return *this;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetProtocol() const
	{
		for (auto beginIt = begin(), it = beginIt, endIt = end(); it != endIt; ++it)
		{
			if (*it == ':' && it + 2 < endIt && *(it + 1) == '/' && *(it + 2) == '/')
			{
				return TPathView(beginIt, static_cast<SizeType>((it + 3) - beginIt));
			}
		}
		return {};
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetFullDomain() const
	{
		const TPathView protocol = GetProtocol();
		const TPathView remaining{begin() + protocol.GetSize(), SizeType(GetSize() - protocol.GetSize())};
		return remaining.GetFirstPath();
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetFullDomainWithProtocol() const
	{
		const TPathView protocol = GetProtocol();
		const TPathView remaining = TPathView{begin() + protocol.GetSize(), SizeType(GetSize() - protocol.GetSize())}.GetFirstPath();
		if (protocol.HasElements())
		{
			return {begin(), SizeType(protocol.GetSize() + remaining.GetSize())};
		}
		else
		{
			return remaining;
		}
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetPort() const
	{
		const TPathView fullDomain = GetFullDomain();
		for (auto it = fullDomain.end() - 1, endIt = fullDomain.begin() - 1; it != endIt; --it)
		{
			if (*it == ':')
			{
				return TPathView(it, static_cast<SizeType>(endIt - it));
			}
		}
		return {};
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetPath() const
	{
		const TPathView protocol = GetProtocol();
		const TPathView fullDomain = GetFullDomain();
		const TPathView query = GetQueryString();
		return TPathView{
			begin() + protocol.GetSize() + fullDomain.GetSize() + 1,
			SizeType(GetSize() - protocol.GetSize() - query.GetSize() - fullDomain.GetSize() - 1)
		};
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetQueryString() const
	{
		if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
		{
			for (auto it = end() - 1, endIt = begin() - 1; it != endIt; --it)
			{
				if (*it == '?')
				{
					return TPathView(it, static_cast<SizeType>(end() - it));
				}
			}
		}

		return {};
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::HasQueryString() const
	{
		if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
		{
			return GetStringView().Contains('?');
		}
		else
		{
			return {};
		}
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetQueryParameterValue(const TPathView parameter) const
	{
		if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
		{
			const TPathView queryString = GetQueryString();

			SizeType index = 0;
			for (auto beginIt = queryString.begin(), it = beginIt, endIt = queryString.end(); it != endIt; ++it)
			{
				if (*it == parameter[index])
				{
					if (++index == parameter.GetSize())
					{
						if (it + 1 == endIt)
						{
							return {it, SizeType(endIt - it)};
						}
						else if (*(it + 1) == '=')
						{
							it += 2;
							auto parameterEndIt = it;
							for (; parameterEndIt < endIt && (*parameterEndIt != '&'); ++parameterEndIt)
								;
							return {it, SizeType(parameterEndIt - it)};
						}
						else
						{
							index = 0;
						}
					}
				}
				else
				{
					index = 0;
				}
			}
		}

		return {};
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] bool TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::HasQueryParameter(const TPathView parameter) const
	{
		if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
		{
			const TPathView queryString = GetQueryString();

			SizeType index = 0;
			for (auto beginIt = queryString.begin(), it = beginIt, endIt = queryString.end(); it != endIt; ++it)
			{
				if (*it == parameter[index])
				{
					if (++index == parameter.GetSize())
					{
						return true;
					}
				}
				else
				{
					index = 0;
				}
			}
		}

		return false;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	[[nodiscard]] TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>
	TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetFragment() const
	{
		if constexpr ((PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries)
		{
			for (auto it = end() - 1, endIt = begin() - 1; it != endIt; --it)
			{
				if (*it == '#')
				{
					return TPathView(it, static_cast<SizeType>(end() - it));
				}
			}
		}

		return {};
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength>
	uint16 TPathView<CharType, Flags, PathSeparator_, MaximumPathLength>::GetDepth() const
	{
		uint16 depth = 0;
		TPathView path = *this;
		while (path.HasElements())
		{
			path = path.GetParentPath();
			depth++;
		}
		return depth;
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	void TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::ReplaceAllExtensions(const ViewType newExtension)
	{
		const SizeType firstExtensionIndex = GetView().FindFirstOf(CharType('.'));
		m_path.TrimNumberOfTrailingCharacters(m_path.GetSize() - firstExtensionIndex);

		m_path += ConstStringViewType(newExtension.begin(), newExtension.end());
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	void TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::ReplaceFileNameWithoutExtensions(const ViewType newFileType)
	{
		const TPath<CharType, Flags, PathSeparator_, MaximumPathLength_> extensions(GetAllExtensions());
		const SizeType lastExtensionIndex = GetView().FindLastOf(CharType('.')) + 1;
		m_path.TrimNumberOfTrailingCharacters(m_path.GetSize() - lastExtensionIndex);

		m_path += ConstStringViewType(newFileType.begin(), newFileType.end());
		m_path += ConstStringViewType(extensions.GetView().begin(), extensions.GetView().end());
	}

	bool Path::CreateDirectories() const
	{
		if (!Exists() && HasElements())
		{
			if (!Path(GetParentPath()).CreateDirectories())
			{
				return false;
			}

			return CreateDirectory();
		}
		else
		{
			return true;
		}
	}

	bool Path::CreateDirectory() const
	{
#if PLATFORM_WINDOWS
		return CreateDirectoryW(GetZeroTerminated(), nullptr);
#elif PLATFORM_POSIX
		return mkdir(GetZeroTerminated(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
	}

	Time::Timestamp Path::GetLastModifiedTime() const
	{
		if (!Exists())
		{
			return Time::Timestamp();
		}

#if PLATFORM_WINDOWS
		struct _stat64 result;
		_wstat64(GetZeroTerminated(), &result);

		return Time::Timestamp::FromSeconds(result.st_mtime);
#elif PLATFORM_POSIX
		struct stat result;
		stat(GetZeroTerminated(), &result);

		return Time::Timestamp::FromSeconds(result.st_mtime);
#endif
	}

	bool Path::MoveFileTo(const ConstZeroTerminatedPathView newFileName) const
	{
#if PLATFORM_WINDOWS
		return MoveFileW(GetZeroTerminated(), newFileName);
#elif PLATFORM_POSIX
		return rename(GetZeroTerminated(), newFileName) == 0;
#endif
	}

	bool Path::CopyFileTo(const ConstZeroTerminatedPathView newFileName) const
	{
		const File source(*this, AccessModeFlags::Read | AccessModeFlags::Binary);
		if (source.IsValid())
		{
			const File destination(newFileName, AccessModeFlags::Write | AccessModeFlags::Binary);
			if (destination.IsValid())
			{
				return destination.Write((FileView)source) == (size)source.GetSize();
			}
		}

		return false;
	}

	bool Path::Serialize(const Serialization::Reader serializer)
	{
		const bool success = serializer.SerializeInPlace(m_path);
		if constexpr (PLATFORM_WINDOWS)
		{
			MakeNativeSlashes();
		}
		AdjustLongPathsIfNecessary();
		return success;
	}

	bool Path::Serialize(Serialization::Writer serializer) const
	{
		if constexpr (PLATFORM_WINDOWS)
		{
			Path copy(*this);
			copy.MakeForwardSlashes();
			PathView::ConstStringViewType view = copy.m_path.GetView();
			return serializer.SerializeInPlace(view);
		}
		else
		{
			return serializer.SerializeInPlace(m_path.GetView());
		}
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator_, uint16 MaximumPathLength_>
	void TPath<CharType, Flags, PathSeparator_, MaximumPathLength_>::ResolveAbsolutePath()
	{
		for (auto begin = m_path.begin(), it = begin, end = m_path.end(); it != end; ++it)
		{
			if (*it == '.' && (it + 2) < end && ((*(it + 1) == '.') & (*(it + 2) == PathSeparator)))
			{
				const ViewType parentPath = ViewType(begin, static_cast<SizeType>((it - 1) - begin)).GetParentPath();
				const StringViewType removedView(const_cast<CharType*>(parentPath.end().Get()), (it + 2));
				m_path.Remove(removedView);
				it -= removedView.GetSize() - 3;
			}
		}
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator, uint16 MaximumPathLength>
	bool TPath<CharType, Flags, PathSeparator, MaximumPathLength>::Serialize(const Serialization::Reader serializer)
	{
		return serializer.SerializeInPlace(m_path);
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator, uint16 MaximumPathLength>
	bool TPath<CharType, Flags, PathSeparator, MaximumPathLength>::Serialize(Serialization::Writer serializer) const
	{
		return serializer.SerializeInPlace(m_path.GetView());
	}

	template<typename CharType, uint8 Flags, CharType PathSeparator, uint16 MaximumPathLength>
	bool TPath<CharType, Flags, PathSeparator, MaximumPathLength>::OpenWithAssociatedApplication() const
	{
#if PLATFORM_WINDOWS
		if constexpr (TypeTraits::IsSame<CharType, wchar_t>)
		{
			ShellExecuteW(0, 0, GetZeroTerminated(), 0, 0, SW_SHOW);
		}
		else if constexpr (TypeTraits::IsSame<CharType, char>)
		{
			ShellExecuteA(0, 0, GetZeroTerminated(), 0, 0, SW_SHOW);
		}
		return true;
#elif PLATFORM_APPLE_MACOS
		NSString* pathString = [NSString stringWithUTF8String:GetZeroTerminated()];
		NSURL* url = [NSURL URLWithString:pathString];
		return [[NSWorkspace sharedWorkspace] openURL:url];
#elif PLATFORM_APPLE_IOS || PLATFORM_APPLE_VISIONOS
		NSString* pathString = [NSString stringWithUTF8String:GetZeroTerminated()];
		NSURL* url = [NSURL URLWithString:pathString];
		[[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
		return true;
#elif PLATFORM_WEB
		em_proxying_queue* queue = emscripten_proxy_get_system_queue();
		pthread_t target = emscripten_main_runtime_thread_id();
		if (target == pthread_self())
		{
			PUSH_CLANG_WARNINGS
			DISABLE_CLANG_WARNING("-Wdollar-in-identifier-extension")
			// clang-format off
            EM_ASM(
                {
                    var url = UTF8ToString($0);
                    window.open(url, '_blank').focus();
                },
                GetZeroTerminated().GetData()
            );
			// clang-format on
			POP_CLANG_WARNINGS
			return true;
		}
		else
		{
			TPath path = *this;
			[[maybe_unused]] const bool called = emscripten_proxy_sync(
																						 queue,
																						 target,
																						 [](void* pUserData)
																						 {
																							 IO::Path& path = *reinterpret_cast<IO::Path*>(pUserData);
																							 PUSH_CLANG_WARNINGS
																							 DISABLE_CLANG_WARNING("-Wdollar-in-identifier-extension")
																							 // clang-format off
                        EM_ASM(
                            {
                                var url = UTF8ToString($0);
                                window.open(url, '_blank').focus();
                            },
                            path.GetZeroTerminated().GetData()
                        );
																							 // clang-format on
																							 POP_CLANG_WARNINGS
																						 },
																						 &path
																					 ) == 1;
			return true;
		}
#else
		Assert(false, "Platform does not support opening URIs");
		return false;
#endif
	}

	template struct TPathView<PathCharType, uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}), PathSeparator, MaximumPathLength>;
	template struct TPathView<
		const PathCharType,
		uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}),
		PathSeparator,
		MaximumPathLength>;
	template struct TPathView<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;
	template struct TPathView<const URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;

	template struct TPath<PathCharType, uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}), PathSeparator, MaximumPathLength>;
	template struct TPath<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;
}

#pragma once

#include <Common/IO/TPath.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::IO
{
	extern template struct TPath<
		PathCharType,
		uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}),
		PathSeparator,
		MaximumPathLength>;

	struct TRIVIAL_ABI Path
		: public TPath<PathCharType, uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}), PathSeparator, MaximumPathLength>
	{
		using TPath::TPath;

		Path(const TPath& path)
			: TPath(path)
		{
		}
		Path& operator=(const TPath& path)
		{
			TPath::operator=(path);
			return *this;
		}
		Path(TPath&& path)
			: TPath(Forward<TPath>(path))
		{
		}
		Path& operator=(TPath&& path)
		{
			TPath::operator=(Forward<TPath>(path));
			return *this;
		}

		bool CreateDirectories() const;
		bool CreateDirectory() const;

		bool MoveFileTo(const ConstZeroTerminatedPathView newFileName) const;
		bool CopyFileTo(const ConstZeroTerminatedPathView newFileName) const;
		bool MoveDirectoryTo(const ConstZeroTerminatedPathView newPath) const;
		bool CopyDirectoryTo(const ConstZeroTerminatedPathView newPath) const;

		[[nodiscard]] Time::Timestamp GetLastModifiedTime() const;

		bool RemoveFile() const;
		bool RemoveDirectory() const;
		bool EmptyDirectoryRecursively() const;

		[[nodiscard]] bool CreateSymbolicLinkToThis(const IO::Path& symbolicLinkPath) const;

		[[nodiscard]] static Path GetExecutablePath();
		[[nodiscard]] static Path GetExecutableDirectory();
		[[nodiscard]] static Path GetWorkingDirectory();
		static bool SetWorkingDirectory(const ConstZeroTerminatedPathView path);
		static void InitializeDataDirectories();
		[[nodiscard]] static bool ClearApplicationDataDirectory();
		[[nodiscard]] static Path GetApplicationDataDirectory();
		[[nodiscard]] static bool ClearApplicationCacheDirectory();
		[[nodiscard]] static Path GetApplicationCacheDirectory();
		[[nodiscard]] static Path GetTemporaryDirectory();
		[[nodiscard]] static Path GetUserDataDirectory();
		[[nodiscard]] static Path GetHomeDirectory();
		[[nodiscard]] static Path GetDownloadsDirectory();

		[[nodiscard]] bool IsDirectory() const;
		[[nodiscard]] bool IsFile() const;
		[[nodiscard]] bool IsSymbolicLink() const;
		[[nodiscard]] bool IsDirectoryEmpty() const;
		[[nodiscard]] Path GetSymbolicLinkTarget() const;

		[[nodiscard]] bool Exists() const;
		[[nodiscard]] bool IsRelative() const;
		[[nodiscard]] bool IsAbsolute() const
		{
			return !IsRelative();
		}

		//! Provides a file or directory duplicating this path, provided it does not already exist
		//! If the path exists, a unique name is generated (i.e. "My File 2")
		[[nodiscard]] IO::Path GetDuplicated() const;

		template<typename... Args>
		[[nodiscard]] static Path Combine(const Args&... args)
		{
			return TPath::Combine(args...);
		}

		template<typename... Args>
		[[nodiscard]] static Path Merge(const Args&... args)
		{
			return TPath::Merge(args...);
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		friend Path operator+(const ViewType left, const ViewType right);
		friend Path operator+(const Path& left, const ViewType right);
		friend Path operator+(const ViewType left, const typename ViewType::ConstStringViewType right);
		friend Path operator+(const Path& left, const typename ViewType::ConstStringViewType right);
	};

	[[nodiscard]] inline Path operator+(const typename Path::ViewType left, const typename Path::ViewType right)
	{
		Path result(Memory::Reserve, left.GetSize() + right.GetSize());
		result.m_path += left.GetStringView();
		result.m_path += right.GetStringView();

		return result;
	}

	[[nodiscard]] inline Path operator+(const Path& left, const typename Path::ViewType right)
	{
		Path result(Memory::Reserve, left.GetSize() + right.GetSize());
		result.m_path += left.GetView().GetStringView();
		result.m_path += right.GetStringView();

		return result;
	}

	[[nodiscard]] inline Path operator+(const typename Path::ViewType left, const typename Path::ViewType::ConstStringViewType right)
	{
		Path result(Memory::Reserve, left.GetSize() + right.GetSize());
		result.m_path += left.GetStringView();
		result.m_path += right;

		return result;
	}

	[[nodiscard]] inline Path operator+(const Path& left, const typename Path::ViewType::ConstStringViewType right)
	{
		Path result(Memory::Reserve, left.GetSize() + right.GetSize());
		result.m_path += left.GetView().GetStringView();
		result.m_path += right;

		return result;
	}

	namespace Internal
	{
#if PLATFORM_ANDROID
		[[nodiscard]] inline PURE_STATICS Path& GetCacheDirectory()
		{
			static Path directory;
			return directory;
		}
		[[nodiscard]] inline PURE_STATICS Path& GetAppDataDirectory()
		{
			static Path directory;
			return directory;
		}
#endif
	}
}

#pragma once

#include <Common/IO/Path.h>

namespace ngine::IO
{
	enum class FileType : uint8
	{
		Unknown,
		File,
		Directory
	};

	struct FileIterator
	{
		FileIterator(const ConstZeroTerminatedPathView directory LIFETIME_BOUND);
		~FileIterator();

		void Next();
		[[nodiscard]] PathView GetCurrentFileName() const;
		[[nodiscard]] Path GetCurrentFilePath() const
		{
			return Path::Combine(m_directory.GetView(), GetCurrentFileName());
		}
		[[nodiscard]] ConstZeroTerminatedPathView GetCurrentDirectory() const
		{
			return m_directory;
		}
		FileType GetCurrentFileType() const;
		[[nodiscard]] bool ReachedEnd() const
		{
#if PLATFORM_WINDOWS
			return m_pHandle == nullptr;
#else
			return m_pEntry == nullptr;
#endif
		}

		enum class TraversalResult : uint8
		{
			Continue,
			Break,
			SkipDirectory
		};

		template<typename Callback>
		inline static void TraverseDirectory(const ConstZeroTerminatedPathView directory, Callback&& callback)
		{
			for (IO::FileIterator fileIterator(directory); !fileIterator.ReachedEnd(); fileIterator.Next())
			{
				if (callback(Forward<IO::Path>(fileIterator.GetCurrentFilePath())) == TraversalResult::Break)
				{
					break;
				}
			}
		}

		template<typename Callback>
		inline static void TraverseDirectoryRecursive(const ConstZeroTerminatedPathView directory, Callback&& callback)
		{
			TraverseDirectoryRecursiveInternal(directory, Forward<Callback>(callback));
		}
	protected:
		template<typename Callback>
		inline static bool TraverseDirectoryRecursiveInternal(const ConstZeroTerminatedPathView directory, Callback&& callback)
		{
			for (IO::FileIterator fileIterator(directory); !fileIterator.ReachedEnd(); fileIterator.Next())
			{
				// Skip internal files
				if (fileIterator.GetCurrentFileName()[0] == MAKE_NATIVE_LITERAL('.'))
				{
					continue;
				}

				const FileType fileType = fileIterator.GetCurrentFileType();
				if (fileType == FileType::File)
				{
					const TraversalResult fileTraversalResult = callback(Forward<IO::Path>(fileIterator.GetCurrentFilePath()));
					if (fileTraversalResult == TraversalResult::Break)
					{
						return false;
					}
					else if (fileTraversalResult == TraversalResult::SkipDirectory)
					{
						break;
					}
				}
				else if (fileType == FileType::Directory)
				{
					const TraversalResult directoryTraversalResult = callback(Forward<IO::Path>(fileIterator.GetCurrentFilePath()));
					if (directoryTraversalResult == TraversalResult::Break)
					{
						return false;
					}
					else if (directoryTraversalResult == TraversalResult::SkipDirectory)
					{
						break;
					}

					if (!TraverseDirectoryRecursiveInternal(fileIterator.GetCurrentFilePath(), Callback(callback)))
					{
						return false;
					}
				}
			}

			return true;
		}
	protected:
		ConstZeroTerminatedPathView m_directory;

#if PLATFORM_WINDOWS
		void* m_pHandle = nullptr;
		char m_findDataBuffer[592];
#elif PLATFORM_POSIX
		void* m_pDirectory = nullptr;
		void* m_pEntry = nullptr;
#endif
	};
}

#include <Common/IO/FileIterator.h>

#if PLATFORM_WINDOWS
#include "Platform/Windows.h"
#elif PLATFORM_POSIX
#include <dirent.h>
#include <cstring>
#endif

#include <Common/IO/Path.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Math/Select.h>

namespace ngine::IO
{
	FileIterator::FileIterator(const ConstZeroTerminatedPathView directory)
		: m_directory(directory)
	{
#if PLATFORM_WINDOWS
		static_assert(sizeof(m_findDataBuffer) == sizeof(WIN32_FIND_DATAW));

		const IO::Path query = Path::Combine(directory.GetView(), MAKE_NATIVE_LITERAL('*'));
		m_pHandle = FindFirstFileW(query.GetZeroTerminated(), reinterpret_cast<WIN32_FIND_DATAW*>(&m_findDataBuffer));
		if (m_pHandle == INVALID_HANDLE_VALUE)
		{
			m_pHandle = nullptr;
			return;
		}

		const WIN32_FIND_DATAW& __restrict findData = *reinterpret_cast<const WIN32_FIND_DATAW*>(&m_findDataBuffer);
		if ((findData.cFileName[0] == '.') & ((findData.cFileName[1] == '\0') | (findData.cFileName[1] == '.')))
		{
			Next();
		}
#elif PLATFORM_POSIX
		if ((m_pDirectory = opendir(directory)))
		{
			Next();
		}
#else
#error "File iterator not implemented for platform"
#endif
	}

	FileIterator::~FileIterator()
	{
#if PLATFORM_WINDOWS
		if (m_pHandle != nullptr)
		{
			FindClose(m_pHandle);
		}
#elif PLATFORM_POSIX
		if (m_pDirectory != nullptr)
		{
			closedir(static_cast<DIR*>(m_pDirectory));
		}
#endif
	}

	void FileIterator::Next()
	{
#if PLATFORM_WINDOWS
		Expect(m_pHandle != nullptr);

		if (!FindNextFileW(m_pHandle, reinterpret_cast<WIN32_FIND_DATAW*>(&m_findDataBuffer)))
		{
			FindClose(m_pHandle);
			m_pHandle = nullptr;
			return;
		}

		const WIN32_FIND_DATAW& __restrict findData = *reinterpret_cast<const WIN32_FIND_DATAW*>(&m_findDataBuffer);
		if ((findData.cFileName[0] == '.') & ((findData.cFileName[1] == '\0') | (findData.cFileName[1] == '.')))
		{
			Next();
		}
#elif PLATFORM_POSIX
		Expect(m_pDirectory != nullptr);

		if (dirent* pEntry = readdir(static_cast<DIR*>(m_pDirectory)))
		{
#if PLATFORM_ANDROID || PLATFORM_EMSCRIPTEN || PLATFORM_LINUX
			PathView name(pEntry->d_name, (PathView::SizeType)strlen(pEntry->d_name));
#else
			PathView name(pEntry->d_name, pEntry->d_namlen);
#endif
			while ((pEntry != nullptr) & ((name == MAKE_PATH("..")) | (name == MAKE_PATH("."))))
			{
				pEntry = readdir(static_cast<DIR*>(m_pDirectory));

				if (pEntry != nullptr)
				{
#if PLATFORM_ANDROID || PLATFORM_EMSCRIPTEN || PLATFORM_LINUX
					name = PathView(pEntry->d_name, (PathView::SizeType)strlen(pEntry->d_name));
#else
					name = PathView(pEntry->d_name, pEntry->d_namlen);
#endif
				}
				else
				{
					name = PathView();
				}
			}

			m_pEntry = pEntry;
		}
		else
		{
			m_pEntry = nullptr;
		}
#endif
	}

	PathView FileIterator::GetCurrentFileName() const
	{
#if PLATFORM_WINDOWS
		Expect(m_pHandle != nullptr);
		const WIN32_FIND_DATAW& __restrict findData = *reinterpret_cast<const WIN32_FIND_DATAW*>(&m_findDataBuffer);

		return PathView(findData.cFileName, static_cast<uint16>(wcslen(findData.cFileName)));
#elif PLATFORM_POSIX
		Expect(m_pEntry != nullptr);
		dirent* pEntry = reinterpret_cast<dirent*>(m_pEntry);
#if PLATFORM_ANDROID || PLATFORM_EMSCRIPTEN || PLATFORM_LINUX
		return PathView(pEntry->d_name, (PathView::SizeType)strlen(pEntry->d_name));
#else
		return PathView(pEntry->d_name, pEntry->d_namlen);
#endif
#endif
	}

	FileType FileIterator::GetCurrentFileType() const
	{
#if PLATFORM_WINDOWS
		Expect(m_pHandle != nullptr);
		const WIN32_FIND_DATAW& __restrict findData = *reinterpret_cast<const WIN32_FIND_DATAW*>(&m_findDataBuffer);

		return Math::Select((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, FileType::Directory, FileType::File);
#elif PLATFORM_POSIX
		Expect(m_pEntry != nullptr);
		dirent* pEntry = reinterpret_cast<dirent*>(m_pEntry);

		switch (pEntry->d_type)
		{
			case 0x8:
				return FileType::File;
			case 0x4:
				return FileType::Directory;
			default:
				return FileType::Unknown;
		}
#endif
	}
}

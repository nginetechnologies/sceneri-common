#include "IO/File.h"

#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/FlatString.h>
#include <Common/Memory/CountBits.h>
#include <Common/EnumFlags.h>
#include <Common/Math/Select.h>
#include <Common/Threading/Sleep.h>
#include "IO/Path.h"

#include <cstdio>
#include <stdio.h>
#include <errno.h>

#if PLATFORM_POSIX
#include <sys/stat.h>
#endif

#if PLATFORM_WINDOWS
#include <io.h>

#include <Platform/Windows.h>
#elif PLATFORM_ANDROID
#include <android/asset_manager.h>
#elif PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/threading.h>
#endif

#define FILE_ACCESS_DEBUG 0

#if FILE_ACCESS_DEBUG
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Threading/Mutexes/Mutex.h>
#include <Common/Threading/Mutexes/SharedMutex.h>
#include <Common/Memory/UniquePtr.h>
#endif

#include <Common/Threading/Atomics/CompareExchangeStrong.h>
#include <Common/Threading/Atomics/Load.h>

namespace ngine::IO
{
	inline static constexpr uint8 MaximumFileAccessStringLength = 5;

	template<typename CharType>
	using FileAccessStringType = TFlatString<CharType, MaximumFileAccessStringLength, Memory::VectorFlags::AllowResize>;

	template<typename CharType>
	FileAccessStringType<CharType> GetFileAccessString(EnumFlags<AccessModeFlags> flags)
	{
		FileAccessStringType<CharType> result(Memory::Reserve, MaximumFileAccessStringLength - 1u);

		if (flags.IsSet(AccessModeFlags::Append))
		{
			Assert(!flags.IsSet(AccessModeFlags::Read | AccessModeFlags::Write));
			result += 'a';
		}
		else if (flags.IsSet(AccessModeFlags::Read) & flags.IsSet(AccessModeFlags::Write))
		{
			result += "w+";
		}
		else if (flags.IsSet(AccessModeFlags::Read))
		{
			result += 'r';
		}
		else if (flags.IsSet(AccessModeFlags::Write))
		{
			result += 'w';
		}

		if (flags.IsSet(AccessModeFlags::Binary))
		{
			result += 'b';
		}

		return result;
	}

#if FILE_ACCESS_DEBUG
	struct MutexInfo
	{
		Threading::SharedMutex mutex;
		bool m_isExclusive = false;
	};

	static Threading::SharedMutex mapMutex;
	static UnorderedMap<IO::Path, UniquePtr<MutexInfo>, IO::Path::Hash> fileMutexes;
#endif

	[[maybe_unused]] inline static constexpr uint8 SleepOnFailAttempts = 3;

#if PLATFORM_WINDOWS
	inline FILE*
	FOpenShared(const ConstZeroTerminatedPathView filePath, const EnumFlags<AccessModeFlags> flags, const SharingFlags sharingFlags)
	{
		const FileAccessStringType<wchar_t> accessMode = GetFileAccessString<wchar_t>(flags);

#if FILE_ACCESS_DEBUG
		MutexInfo* pFileMutex;
		{
			{
				Threading::SharedLock mapLock(mapMutex);
				auto it = fileMutexes.Find(IO::Path(filePath));
				if (it != fileMutexes.end())
				{
					pFileMutex = it->second.Get();
				}
				else
				{
					mapLock.Unlock();
					Threading::UniqueLock mapWriteLock(mapMutex);
					it = fileMutexes.Find(IO::Path(filePath));
					if (it != fileMutexes.end())
					{
						pFileMutex = it->second.Get();
					}
					else
					{
						pFileMutex = fileMutexes.Emplace(IO::Path(filePath), UniquePtr<MutexInfo>::Make())->second.Get();
					}
				}
			}
		}

		Assert(pFileMutex->mutex.TryLockShared());
		pFileMutex->m_isExclusive = false;
#endif

		FILE* pFile;
		uint8 remainingAttempts = SleepOnFailAttempts;
		while (true)
		{
			pFile = ::_wfsopen(filePath, accessMode.GetZeroTerminated(), static_cast<int>(sharingFlags));
			if (pFile != nullptr)
			{
				return pFile;
			}

			const int error = errno;
			if ((error != EMFILE && error != ENFILE) && (--remainingAttempts == 0 || error != EACCES))
			{
				break;
			}

			Threading::Sleep(0);
		}

#if FILE_ACCESS_DEBUG
		pFileMutex->mutex.UnlockShared();
#endif
		return nullptr;
	}
#endif // #if PLATFORM_WINDOWS

#if PLATFORM_ANDROID
	static int android_read(void* userData, char* buf, int size)
	{
		return AAsset_read((AAsset*)userData, buf, size);
	}

	static int android_write([[maybe_unused]] void* userData, [[maybe_unused]] const char* buf, [[maybe_unused]] int size)
	{
		return EACCES; // can't provide write access to the apk
	}

	static fpos_t android_seek(void* userData, fpos_t offset, int whence)
	{
		return AAsset_seek((AAsset*)userData, offset, whence);
	}

	static int android_close(void* userData)
	{
		AAsset_close((AAsset*)userData);
		return 0;
	}

	FILE* FOpen(const char* filePath, const EnumFlags<AccessModeFlags> flags)
	{
		const FileAccessStringType<char> accessMode = GetFileAccessString<char>(flags);

		FILE* pFile;
		uint8 remainingAttempts = SleepOnFailAttempts;
		while (true)
		{
			if (flags.IsSet(AccessModeFlags::Write))
			{
				pFile = ::fopen(filePath, accessMode.GetZeroTerminated());
			}
			else if (AAsset* asset = AAssetManager_open(Internal::GetAndroidAssetManager(), filePath, 0))
			{
				pFile = funopen(asset, android_read, android_write, android_seek, android_close);
			}
			else
			{
				pFile = ::fopen(filePath, accessMode.GetZeroTerminated());
			}
			if (pFile != nullptr)
			{
				return pFile;
			}

			const int error = errno;
			if ((error != EMFILE && error != ENFILE) && (--remainingAttempts == 0 || error != EACCES))
			{
				break;
			}

			Threading::Sleep(0);
		}
		return nullptr;
	}
#elif PLATFORM_EMSCRIPTEN
	struct FileCookie
	{
		~FileCookie()
		{
			if (pData != nullptr)
			{
				free(pData);
			}
		}

		void* pData{nullptr};
		int dataSize{0};
		int errorCode{0};
		off64_t location{0};
	};

	static ssize_t emscripten_read(void* pUserData, char* buf, size_t readSize)
	{
		FileCookie* __restrict pCookie = reinterpret_cast<FileCookie*>(pUserData);

		const ByteView target{reinterpret_cast<ByteType*>(buf), readSize};
		const ConstByteView source{
			reinterpret_cast<ByteType*>(pCookie->pData) + pCookie->location,
			static_cast<size>(pCookie->dataSize - pCookie->location)
		};
		target.CopyFrom(source);
		const ssize_t copied = Math::Min(target.GetDataSize(), source.GetDataSize());
		pCookie->location += copied;
		return copied;
	}

	static ssize_t emscripten_write([[maybe_unused]] void* pUserData, [[maybe_unused]] const char* buf, [[maybe_unused]] size size)
	{
		return EACCES; // can't provide write access
	}

	static int emscripten_seek(void* pUserData, off64_t* offset, int whence)
	{
		FileCookie* __restrict pCookie = reinterpret_cast<FileCookie*>(pUserData);
		off64_t newOffset;
		switch (whence)
		{
			case SEEK_SET:
				newOffset = *offset;
				break;
			case SEEK_CUR:
				newOffset = pCookie->location + *offset;
				break;
			case SEEK_END:
				newOffset = pCookie->dataSize + *offset;
				break;
			default:
				errno = EINVAL;
				return -1;
		}
		if (newOffset > pCookie->dataSize || (off64_t)newOffset < 0)
		{
			return -1;
		}
		*offset = newOffset;
		pCookie->location = newOffset;
		return 0;
	}

	static int emscripten_close(void* pUserData)
	{
		FileCookie* __restrict pCookie = reinterpret_cast<FileCookie*>(pUserData);
		delete pCookie;
		return 0;
	}

	FILE* FOpen(const ConstZeroTerminatedStringView filePath, const EnumFlags<AccessModeFlags> flags)
	{
		const FileAccessStringType<char> accessMode = GetFileAccessString<char>(flags);
		if (FILE* pFile = ::fopen(filePath, accessMode.GetZeroTerminated()))
		{
			return pFile;
		}
		else if (flags.IsSet(AccessModeFlags::Read))
		{
			if(filePath.GetView().StartsWith(IO::Path::GetApplicationDataDirectory().GetView().GetStringView())
				||filePath.GetView().StartsWith(IO::Path::GetApplicationCacheDirectory().GetView().GetStringView())
				||filePath.GetView().StartsWith(IO::Path::GetTemporaryDirectory().GetView().GetStringView())
				||filePath.GetView().StartsWith(IO::Path::GetUserDataDirectory().GetView().GetStringView()))
			{
				// Early out for local only paths that can't exist from network calls
				return nullptr;
			}

			// Attempt to load from network
			FileCookie* pCookie = new FileCookie();
			emscripten_async_wget_data(
				filePath,
				pCookie,
				[](void* pUserData, void* pData, const int dataSize) // File loaded
				{
					FileCookie& fileCookie = *reinterpret_cast<FileCookie*>(pUserData);
					if (dataSize == 0)
					{
						int expected{0};
						Threading::Atomics::CompareExchangeStrong(fileCookie.errorCode, expected, 1);
						return;
					}

					fileCookie.pData = malloc(dataSize);
					int expected{0};
					Threading::Atomics::CompareExchangeStrong(fileCookie.dataSize, expected, dataSize);
					Memory::CopyNonOverlappingElements(
						reinterpret_cast<ByteType*>(fileCookie.pData),
						reinterpret_cast<const ByteType*>(pData),
						dataSize
					);
				},
				[](void* pUserData) // Error
				{
					FileCookie& fileCookie = *reinterpret_cast<FileCookie*>(pUserData);
					int expected{0};
					Threading::Atomics::CompareExchangeStrong(fileCookie.errorCode, expected, 1);
				}
			);

			while (Threading::Atomics::Load(pCookie->dataSize) == 0 && Threading::Atomics::Load(pCookie->errorCode) == 0)
				;

			if (pCookie->errorCode == 0)
			{
				return fopencookie(
					pCookie,
					accessMode.GetZeroTerminated(),
					cookie_io_functions_t{emscripten_read, emscripten_write, emscripten_seek, emscripten_close}
				);
			}
			else
			{
				delete pCookie;
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}
	}
#elif PLATFORM_WINDOWS
	inline FILE* FOpen(const IO::ConstZeroTerminatedPathView filePath, const EnumFlags<AccessModeFlags> flags)
	{
		if (flags.IsSet(AccessModeFlags::Read))
		{
			return FOpenShared(filePath, flags, IO::SharingFlags::DisallowWrite);
		}

		const FileAccessStringType<wchar_t> accessMode = GetFileAccessString<wchar_t>(flags);

#if FILE_ACCESS_DEBUG
		MutexInfo* pFileMutex;
		{
			{
				Threading::SharedLock mapLock(mapMutex);
				auto it = fileMutexes.Find(IO::Path(filePath));
				if (it != fileMutexes.end())
				{
					pFileMutex = it->second.Get();
				}
				else
				{
					mapLock.Unlock();
					Threading::UniqueLock mapWriteLock(mapMutex);
					it = fileMutexes.Find(IO::Path(filePath));
					if (it != fileMutexes.end())
					{
						pFileMutex = it->second.Get();
					}
					else
					{
						pFileMutex = fileMutexes.Emplace(IO::Path(filePath), UniquePtr<MutexInfo>::Make())->second.Get();
					}
				}
			}
		}

		Assert(pFileMutex->mutex.TryLockExclusive());
		pFileMutex->m_isExclusive = true;
#endif

		FILE* pFile;
		errno_t errorCode;
		uint8 remainingAttempts = SleepOnFailAttempts;
		while (true)
		{
			errorCode = ::_wfopen_s(&pFile, filePath, accessMode.GetZeroTerminated());
			if (errorCode == 0)
			{
				return pFile;
			}

			const int error = errno;
			if ((error != EMFILE && error != ENFILE) && (--remainingAttempts == 0 || error != EACCES))
			{
				break;
			}

			Threading::Sleep(0);
		}
#if FILE_ACCESS_DEBUG
		pFileMutex->mutex.UnlockExclusive();
#endif
		return nullptr;
	}
#else
	FILE* FOpen(const char* filePath, const EnumFlags<AccessModeFlags> flags)
	{
		const FileAccessStringType<char> accessMode = GetFileAccessString<char>(flags);

		FILE* pFile;
		uint8 remainingAttempts = SleepOnFailAttempts;
		while (true)
		{
			pFile = fopen(filePath, accessMode.GetZeroTerminated());
			if (pFile != nullptr)
			{
				return pFile;
			}

			const int error = errno;
			if ((error != EMFILE && error != ENFILE) && (--remainingAttempts == 0 || error != EACCES))
			{
				break;
			}

			Threading::Sleep(0);
		}
		return nullptr;
	}
#endif

	File::File(const IO::ConstZeroTerminatedPathView filePath, const EnumFlags<AccessModeFlags> flags)
		: FileView(FOpen(filePath, flags))
	{
	}

	File::File(
		const IO::ConstZeroTerminatedPathView filePath,
		const EnumFlags<AccessModeFlags> flags,
		[[maybe_unused]] const IO::SharingFlags sharingFlags
	)
#if PLATFORM_WINDOWS
		: FileView(FOpenShared(filePath, flags, sharingFlags))
#else
		// Other platforms are shared by default
		: FileView(FOpen(filePath, flags))
#endif
	{
	}

	File::~File()
	{
		if (m_pFile != nullptr)
		{
			Close();
		}
	}

	size_t FileView::Write(const void* pBuffer, const size_t elementSize, const size_t count) const
	{
#if PLATFORM_WINDOWS
		return _fwrite_nolock(pBuffer, elementSize, count, static_cast<FILE*>(m_pFile));
#else
		return fwrite(pBuffer, elementSize, count, static_cast<FILE*>(m_pFile));
#endif
	}

	int FileView::WriteCharacter(const int character) const
	{
#if PLATFORM_WINDOWS
		return _fputc_nolock(character, static_cast<FILE*>(m_pFile));
#else
		return fputc(character, static_cast<FILE*>(m_pFile));
#endif
	}

	size_t FileView::Read(void* pBuffer, const size_t elementSize, const size_t count) const
	{
#if PLATFORM_WINDOWS
		return _fread_nolock(pBuffer, elementSize, count, static_cast<FILE*>(m_pFile));
#else
		return fread(pBuffer, elementSize, count, static_cast<FILE*>(m_pFile));
#endif
	}

	bool FileView::ReadLineIntoView(const ArrayView<char, uint32> data) const
	{
		return fgets(data.GetData(), (int)data.GetSize(), static_cast<FILE*>(m_pFile)) != nullptr;
	}

	File::SizeType FileView::GetSize() const
	{
		if (m_pFile == nullptr)
		{
			return 0;
		}

#if PLATFORM_WINDOWS
		HANDLE hFile = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(static_cast<FILE*>(m_pFile))));

		LARGE_INTEGER filesize = {0, 0};
		GetFileSizeEx(hFile, &filesize);
		return filesize.QuadPart;
#elif PLATFORM_POSIX && !PLATFORM_ANDROID
		const int fileDescriptor = fileno(static_cast<FILE*>(m_pFile));
		if (fileDescriptor > 0)
		{
			struct stat stbuf;
			if (LIKELY(fstat(fileDescriptor, &stbuf) == 0) && (S_ISREG(stbuf.st_mode)))
			{
				return stbuf.st_size;
			}
		}
#endif

#if !PLATFORM_WINDOWS
		const SizeType position = Tell();
		if (LIKELY(Seek(0, static_cast<SeekOrigin>(SEEK_END))))
		{
			const SizeType size = Tell();
			if (LIKELY(Seek((long)position, SeekOrigin::StartOfFile)))
			{
				Assert(size >= 0);
				return size;
			}
		}
		return 0;
#endif
	}

	File::SizeType FileView::Tell() const
	{
		Expect(m_pFile != nullptr);
#if PLATFORM_WINDOWS
		return _ftelli64_nolock(static_cast<FILE*>(m_pFile));
#else
		return ftell(static_cast<FILE*>(m_pFile));
#endif
	}

	bool FileView::Seek(const long offset, const SeekOrigin origin) const
	{
		Expect(m_pFile != nullptr);

#if PLATFORM_WINDOWS
		return _fseeki64_nolock(static_cast<FILE*>(m_pFile), offset, static_cast<int>(origin)) == 0;
#else
		return fseek(static_cast<FILE*>(m_pFile), offset, static_cast<int>(origin)) == 0;
#endif
	}

	void FileView::Flush() const
	{
		Expect(m_pFile != nullptr);
#if PLATFORM_WINDOWS
		_fflush_nolock(static_cast<FILE*>(m_pFile));
#else
		fflush(static_cast<FILE*>(m_pFile));
#endif
	}

	void File::Close()
	{
		Expect(m_pFile != nullptr);
#if PLATFORM_WINDOWS
		_fclose_nolock(static_cast<FILE*>(m_pFile));
#else
		fclose(static_cast<FILE*>(m_pFile));
#endif
		m_pFile = nullptr;

#if FILE_ACCESS_DEBUG
		{
			MutexInfo* pMutex;
			{
				Threading::SharedLock mapLock(mapMutex);
				auto it = fileMutexes.Find(filePath);
				Assert(it != fileMutexes.end());
				pMutex = it->second.Get();
			}
			if (pMutex->m_isExclusive)
			{
				pMutex->mutex.UnlockExclusive();
			}
			else
			{
				pMutex->mutex.UnlockShared();
			}
		}
#endif
	}
}

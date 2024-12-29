#pragma once

#include "FileView.h"

#include "AccessModeFlags.h"
#include "SharingFlags.h"

#include <Common/ForwardDeclarations/EnumFlags.h>
#include <Common/IO/ForwardDeclarations/ZeroTerminatedPathView.h>
#include <Common/Platform/TrivialABI.h>

#if PLATFORM_ANDROID
struct AAssetManager;
#endif

namespace ngine::IO
{
#if PLATFORM_ANDROID
	namespace Internal
	{
		[[nodiscard]] inline PURE_STATICS AAssetManager*& GetAndroidAssetManager()
		{
			static AAssetManager* pAndroidAssetManager;
			return pAndroidAssetManager;
		}
	}
#endif

	struct [[nodiscard]] TRIVIAL_ABI File : public FileView
	{
		using SizeType = FileView::SizeType;

		File() = default;
		File(void* pFile)
			: FileView(pFile)
		{
		}
		File(const IO::ConstZeroTerminatedPathView filePath, const EnumFlags<AccessModeFlags> flags);
		File(const IO::ConstZeroTerminatedPathView filePath, const EnumFlags<AccessModeFlags> flags, const IO::SharingFlags sharingFlags);
		File(const File&) = delete;
		File& operator=(const File&) = delete;
		File(File&& other) noexcept
		{
			m_pFile = other.m_pFile;
			other.m_pFile = nullptr;
		}
		File& operator=(File&& other) noexcept
		{
			m_pFile = other.m_pFile;
			other.m_pFile = nullptr;
			return *this;
		}
		~File();

		void Close();
	};
}

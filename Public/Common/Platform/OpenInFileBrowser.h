#pragma once

#if PLATFORM_DESKTOP

#include <Common/IO/Path.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/IO/Format/Path.h>
#include <iostream>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#endif

namespace ngine::Platform
{
#if PLATFORM_WINDOWS
	inline bool OpenInFileBrowser(const IO::Path& path)
	{
		if (path.IsFile())
		{
			PIDLIST_ABSOLUTE pItemIdList = ILCreateFromPathW(path.GetZeroTerminated());
			if (pItemIdList != nullptr)
			{
				const bool success = SHOpenFolderAndSelectItems(pItemIdList, 0, 0, 0) == S_OK;
				ILFree(pItemIdList);
				return success;
			}
			return false;
		}
		else
		{
			// Open the directory itself
			return (INT_PTR)ShellExecuteW(nullptr, L"open", path.GetZeroTerminated(), nullptr, nullptr, SW_SHOWDEFAULT) > 32;
		}
	}
#elif PLATFORM_APPLE_MACOS || PLATFORM_APPLE_MACCATALYST
	bool OpenInFileBrowser(const IO::Path& path);
#elif PLATFORM_LINUX
inline bool OpenInFileBrowser(const IO::Path& path)
{
	NativeString command;
	command.Format("xdg-open {}", path);
	system(command.GetZeroTerminated());
	return true;
}
#endif
}

#endif

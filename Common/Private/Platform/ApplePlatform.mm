#include <Common/Platform/OpenInFileBrowser.h>

#if PLATFORM_DESKTOP

#if PLATFORM_APPLE_MACOS || PLATFORM_APPLE_MACCATALYST
#import <AppKit/NSWorkspace.h>
#endif

namespace ngine::Platform
{
#if PLATFORM_APPLE_MACOS || PLATFORM_APPLE_MACCATALYST
	__attribute__((weak)) bool OpenInFileBrowser(const IO::Path& path)
	{
#if PLATFORM_APPLE_MACOS
		NSString* pathString = [NSString stringWithUTF8String:path.GetZeroTerminated()];
		if (path.IsFile())
		{
			NSURL* url = [NSURL fileURLWithPath:pathString];
			NSArray* fileURLs = [NSArray arrayWithObjects:url, nil];
			[[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
			return true;
		}
		else
		{
			[[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:pathString];
			return false;
		}
#elif PLATFORM_APPLE_MACCATALYST
		UNUSED(path);
		Assert(false, "Not supported");
		return false;
#endif
	}
#endif
}

#endif

#include "IO/Library.h"

#include "IO/Path.h"

#include <Common/Memory/Containers/Format/String.h>

#if PLATFORM_WINDOWS
#include "Platform/Windows.h"
#elif PLATFORM_APPLE
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#include <dlfcn.h>
#elif PLATFORM_POSIX
#include <dlfcn.h>
#endif

namespace ngine::IO
{
#if PLATFORM_APPLE
	void* LoadBundle(const IO::ConstZeroTerminatedPathView path)
	{
		NSString* pathString = [NSString stringWithUTF8String:path];
		NSURL* bundleURL = [NSURL fileURLWithPath:pathString];
		return CFBundleCreate(kCFAllocatorDefault, (CFURLRef)bundleURL);
	}
#endif

	Library::Library(const IO::ConstZeroTerminatedPathView path)
#if PLATFORM_WINDOWS
		: LibraryView(::LoadLibraryW(path))
#elif PLATFORM_APPLE
		: LibraryView(LoadBundle(path))
#elif PLATFORM_POSIX
		: LibraryView(::dlopen(path, RTLD_NOW))
#else
		Assert(false, "Library opening not supported on platform!");
#endif
	{
#if PLATFORM_POSIX
		if constexpr (ENABLE_ASSERTS)
		{
			[[maybe_unused]] const char* errorMessage = dlerror();
			AssertMessage(m_pModuleHandle != nullptr, "{}", errorMessage != nullptr ? errorMessage : "Failed to open library");
		}
#endif
	}

	Library::~Library()
	{
#if PLATFORM_WINDOWS
		::FreeLibrary(static_cast<HMODULE>(m_pModuleHandle));
#elif PLATFORM_APPLE
		if (m_pModuleHandle != nullptr)
		{
			CFRelease(m_pModuleHandle);
		}
#elif PLATFORM_POSIX
		if (m_pModuleHandle != nullptr)
		{
			::dlclose(m_pModuleHandle);
		}
#endif
	}

	void* LibraryView::GetProcedureAddressInternal(const ZeroTerminatedStringView name) const
	{
#if PLATFORM_WINDOWS
		return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(m_pModuleHandle), name));
#elif PLATFORM_APPLE
		return CFBundleGetFunctionPointerForName(
			static_cast<CFBundleRef>(m_pModuleHandle),
			CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingMacRoman)
		);
#elif PLATFORM_POSIX
		return ::dlsym(m_pModuleHandle, name);
#endif
	}
}

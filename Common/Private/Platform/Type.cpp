#include <Common/Platform/Type.h>

#if PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif

#if PLATFORM_LINUX
#include <Common/IO/File.h>
#include <Common/IO/Path.h>
#include <Common/EnumFlags.h>

#include <cstring>
#endif

namespace ngine::Platform
{
	PURE_STATICS bool IsMobile()
	{
#if PLATFORM_EMSCRIPTEN

		return EM_ASM_INT({
						 // Don't let clang-format affect JavaScript
			       // clang-format off
						 if(/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)) {
							 // true for mobile device
							 return 1;
						 } else {
							 // false for not mobile device
							 return 0;
						 }
						 // clang-format on
					 }) != 0;
#elif PLATFORM_MOBILE
		return true;
#elif PLATFORM_LINUX
		const IO::Path chassisTypePath{MAKE_PATH("/sys/class/dmi/id/chassis_type")};
		const IO::File file = IO::File(chassisTypePath.GetZeroTerminated(), EnumFlags<IO::AccessModeFlags>{IO::AccessModeFlags::Read});
		if (file.IsValid())
		{
			Array<char, 16> buffer;
			if (file.ReadLineIntoView(buffer.GetDynamicView()))
			{
				const ConstStringView data{buffer.GetData(), (uint32)strlen(buffer.GetData())};
				switch (data.ToIntegral<int32>())
				{
					// Dockable
					case 10:
					// Dockable handheld
					case 14:
						// Handheld or dockable handheld
						return true;
					case 30:
						return true;
					default:
						return false;
				}
			}
		}
		return false;
#else
		return false;
#endif
	}

	PURE_STATICS bool IsDesktop()
	{
#if PLATFORM_EMSCRIPTEN
		return !IsMobile();
#else
		return PLATFORM_DESKTOP;
#endif
	}
}

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
	PURE_STATICS Type GetEffectiveType()
	{
#if PLATFORM_EMSCRIPTEN
		static_assert((uint8)Type::Windows == (1 << 0));
		static_assert((uint8)Type::macOS == (1 << 1));
		static_assert((uint8)Type::iOS == (1 << 2));
		static_assert((uint8)Type::visionOS == (1 << 4));
		static_assert((uint8)Type::Android == (1 << 5));
		static_assert((uint8)Type::Web == (1 << 6));
		static_assert((uint8)Type::Linux == (1 << 7));
		const int platformType = EM_ASM_INT({
			// Don't let clang-format affect JavaScript
			// clang-format off
						const platform = navigator.platform.toUpperCase();
						const ua = navigator.userAgent.toLowerCase();
						if(platform.indexOf('MAC') >= 0) {
							return 1 << 1;
						}
						else if(platform.indexOf('IPHONE') >= 0 || platform.indexOf('IPAD') >= 0 || platform.indexOf('IPOD') >= 0) {
							return 1 << 2;
						}
						else if(platform.indexOf('WIN') >= 0) {
							return 1 << 0;
						}
						else if(platform.indexOf('ANDROID') >= 0) {
							return 1 << 5;
						}
						else if(platform.indexOf('LINUX') >= 0 || platform.indexOf('X11') >= 0) {
							return 1 << 7;
						} else {
							console.log("Unknown platform " + platform);
							return 1 << 6;
						}
			// clang-format on
		});
		Assert((Type)platformType != Type::Web, "Failed to detect underlying web platform!");
		return (Type)platformType;
#else
		return Current;
#endif
	}

	PURE_STATICS Class GetClass()
	{
#if PLATFORM_EMSCRIPTEN
		const bool isMobile = EM_ASM_INT({
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
		return isMobile ? Class::Mobile : Class::Desktop;
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
					// Hand held
					case 11:
					// Tablet
					case 30:
					// Portable
					case 8:
						return Class::Mobile;
					// Desktop
					case 3:
					// Low profile desktop
					case 4:
					// Mini tower
					case 6:
					// Tower
					case 7:
					// Laptop
					case 9:
					// Notebook
					case 10:
					// Docking station
					case 12:
					// All in one
					case 14:
					// Space-saving
					case 15:
					// Convertible
					case 31:
					// Detachable
					case 32:
					// Embedded PC
					case 34:
					// Mini PC
					case 35:
					// Stick PC
					case 36:
					default:
						return Class::Desktop;
				}
			}
		}
		return Class::Desktop;
#elif PLATFORM_DESKTOP
		return Class::Desktop;
#elif PLATFORM_MOBILE
		return Class::Mobile;
#elif PLATFORM_SPATIAL
		return Class::Spatial;
#else
#error "Unknown device class"
#endif
	}
}

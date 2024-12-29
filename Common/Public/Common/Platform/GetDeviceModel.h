#pragma once

#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/Format/String.h>

#if PLATFORM_POSIX
#include <sys/utsname.h>
#if PLATFORM_APPLE
#include <sys/sysctl.h>
#endif
#endif

namespace ngine::Platform
{
#if PLATFORM_POSIX
#if PLATFORM_APPLE
	[[nodiscard]] inline FlatString<_SYS_NAMELEN> GetDeviceModelName()
#else
	[[nodiscard]] inline FlatString<300> GetDeviceModelName()
#endif
	{
#if PLATFORM_APPLE_MACOS || PLATFORM_APPLE_MACCATALYST
		FlatString<_SYS_NAMELEN> result;
		size length = _SYS_NAMELEN;
		::sysctlbyname("hw.model", result.GetData(), &length, nullptr, 0);
		result.Resize((uint16)length - 1, Memory::Uninitialized);
		return result;
#elif PLATFORM_WEB
		return "Web";
#else
		struct utsname systemInfo;
		uname(&systemInfo);
		return {systemInfo.machine, (uint16)strlen(systemInfo.machine)};
#endif
	}
#else
	[[nodiscard]] inline ConstStringView GetDeviceModelName()
	{
#if PLATFORM_WINDOWS
		return "Unknown Windows Device";
#else
		static_unreachable("GetDeviceModelName has not been implemented for this platform!");
#endif
	}
#endif

	enum class DeviceModel : uint16
	{
		UnknownApple,

		UnknowniPhone,
		iPhone4S,
		iPhone5,
		iPhone5C,
		iPhone5S,
		iPhoneSE,
		iPhoneSE2,
		iPhoneSE3,
		iPhone6,
		iPhone6Plus,
		iPhone6S,
		iPhone6SPlus,
		iPhone7,
		iPhone7Plus,
		iPhone8,
		iPhone8Plus,
		iPhoneX,
		iPhoneXS,
		iPhoneXSMax,
		iPhoneXR,
		iPhone11,
		iPhone11Pro,
		iPhone11ProMax,
		iPhone12,
		iPhone12Mini,
		iPhone12Pro,
		iPhone12ProMax,
		iPhone13,
		iPhone13Mini,
		iPhone13Pro,
		iPhone13ProMax,
		iPhone14,
		iPhone14Plus,
		iPhone14Pro,
		iPhone14ProMax,
		iPhone15,
		iPhone15Plus,
		iPhone15Pro,
		iPhone15ProMax,
		iPhone16,
		iPhone16Plus,
		iPhone16Pro,
		iPhone16ProMax,

		UnknowniPad,
		iPadMini,
		iPadMini2,
		iPadMini3,
		iPadMini4,
		iPadMini5,
		iPadMini6,
		iPad2,
		iPad3,
		iPad4,
		iPad5,
		iPad6,
		iPad7,
		iPad8,
		iPad9,
		iPad10,
		iPadAir,
		iPadAir2,
		iPadAir3,
		iPadAir4,
		iPadAir5,
		iPadPro_9_7,
		iPadPro_10_5,
		iPadPro_11,
		iPadPro_11_2nd,
		iPadPro_11_3rd,
		iPadPro_11_4th,
		iPadPro_12_9,
		iPadPro_12_9_2nd,
		iPadPro_12_9_3rd,
		iPadPro_12_9_4th,
		iPadPro_12_9_5th,
		iPadPro_12_9_6th,

		UnknowniPod,
		iPodTouch5,
		iPodTouch6,
		iPodTouch7,

		UnknownMac,

		UnknownMacBookAir,
		MacBookAir11_2012,
		MacBookAir13_2012,
		MacBookAir11_2013,
		MacBookAir13_2013,
		MacBookAir11_2015,
		MacBookAir13_2015,
		MacBookAir13_2018,
		MacBookAir13_2019,
		MacBookAir13_2020,
		MacBookAir13_M1_2020,
		MacBookAir13_M2,

		MacStudio_M1Max,
		MacStudio_M1Ultra,
		MacStudio_M2Max,
		MacStudio_M2Ultra,

		UnknownMacBookPro,
		MacBookPro13_2012,
		MacBookPro15_2012,
		MacBookPro13_2013,
		MacBookPro15_2013,
		MacBookPro13_2014,
		MacBookPro15_2014,
		MacBookPro13_2015,
		MacBookPro15_2015,
		MacBookPro13_2016,
		MacBookPro15_2016,
		MacBookPro13_2017,
		MacBookPro15_2017,
		MacBookPro13_2018,
		MacBookPro15_2018,
		MacBookPro16_2019,
		MacBookPro13_2020,
		MacBookPro13_M1_2020,
		MacBookPro14_2021,
		MacBookPro16_2021,
		MacBookPro13_M2,

		UnknownMacBook,
		MacBook12_2015,
		MacBook12_2016,
		MacBook12_2017,

		UnknownMacMini,
		MacMini_5_2012,
		MacMini_2012,
		MacMini_2014,
		MacMini_2018,
		MacMini_2020,

		UnknowniMac,
		iMac21_5_2012,
		iMac27_2012,
		iMac21_5_2013,
		iMac27_2013,
		iMac21_5_2014,
		iMac27_2014,
		iMac21_5_2015,
		iMac27_2015,
		iMac21_5_2017,
		iMac27_2017,
		iMacPro27_2017,
		iMac21_5_2019,
		iMac27_2019,
		iMac27_2020,
		iMac24_2021,

		VisionPro,
		UnknownVision,

		UnknownWindows,

		UnknownAndroid,

		UnknownWeb,

		UnknownLinux
	};

	[[nodiscard]] inline DeviceModel GetDeviceModel()
	{
#if PLATFORM_APPLE_IOS || PLATFORM_APPLE_MACOS || PLATFORM_APPLE_VISIONOS
		const FlatString<_SYS_NAMELEN> modelString = Platform::GetDeviceModelName();

		if (modelString.GetView().StartsWith("iPhone"))
		{
			if (modelString == "iPhone4,1")
			{
				return DeviceModel::iPhone4S;
			}
			else if (modelString == "iPhone5,1" || modelString == "iPhone5,2")
			{
				return DeviceModel::iPhone5;
			}
			else if (modelString == "iPhone5,3" || modelString == "iPhone5,4")
			{
				return DeviceModel::iPhone5C;
			}
			else if (modelString == "iPhone6,1" || modelString == "iPhone6,2")
			{
				return DeviceModel::iPhone5S;
			}
			else if (modelString == "iPhone8,4")
			{
				return DeviceModel::iPhoneSE;
			}
			else if (modelString == "iPhone7.2")
			{
				return DeviceModel::iPhone6;
			}
			else if (modelString == "iPhone8.1")
			{
				return DeviceModel::iPhone6S;
			}
			else if (modelString == "iPhone9,1" || modelString == "iPhone9,3")
			{
				return DeviceModel::iPhone7;
			}
			else if (modelString == "iPhone10,1" || modelString == "iPhone10,4")
			{
				return DeviceModel::iPhone8;
			}
			else if (modelString == "iPhone12,8")
			{
				return DeviceModel::iPhoneSE2;
			}
			else if (modelString == "iPhone14,6")
			{
				return DeviceModel::iPhoneSE3;
			}
			else if (modelString == "iPhone13,1")
			{
				return DeviceModel::iPhone12Mini;
			}
			else if (modelString == "iPhone14,4")
			{
				return DeviceModel::iPhone13Mini;
			}
			else if (modelString == "iPhone7,1")
			{
				return DeviceModel::iPhone6Plus;
			}
			else if (modelString == "iPhone8,2")
			{
				return DeviceModel::iPhone6SPlus;
			}
			else if (modelString == "iPhone9,2" || modelString == "iPhone9,4")
			{
				return DeviceModel::iPhone7Plus;
			}
			else if (modelString == "iPhone10,2" || modelString == "iPhone10,5")
			{
				return DeviceModel::iPhone8Plus;
			}
			else if (modelString == "iPhone10,3" || modelString == "iPhone10,6")
			{
				return DeviceModel::iPhoneX;
			}
			else if (modelString == "iPhone11,2")
			{
				return DeviceModel::iPhoneXS;
			}
			else if (modelString == "iPhone12,3")
			{
				return DeviceModel::iPhone11Pro;
			}
			else if (modelString == "iPhone11,8")
			{
				return DeviceModel::iPhoneXR;
			}
			else if (modelString == "iPhone12,1")
			{
				return DeviceModel::iPhone11;
			}
			else if (modelString == "iPhone13,2")
			{
				return DeviceModel::iPhone12;
			}
			else if (modelString == "iPhone13,3")
			{
				return DeviceModel::iPhone12Pro;
			}
			else if (modelString == "iPhone14,5")
			{
				return DeviceModel::iPhone13;
			}
			else if (modelString == "iPhone14,2")
			{
				return DeviceModel::iPhone13Pro;
			}
			else if (modelString == "iPhone11,4" || modelString == "iPhone11,6")
			{
				return DeviceModel::iPhoneXSMax;
			}
			else if (modelString == "iPhone12,5")
			{
				return DeviceModel::iPhone11ProMax;
			}
			else if (modelString == "iPhone13,4")
			{
				return DeviceModel::iPhone12ProMax;
			}
			else if (modelString == "iPhone14,3")
			{
				return DeviceModel::iPhone13ProMax;
			}
			else if (modelString == "iPhone14,4")
			{
				return DeviceModel::iPhone13Mini;
			}
			else if (modelString == "iPhone14,7")
			{
				return DeviceModel::iPhone14;
			}
			else if (modelString == "iPhone14,8")
			{
				return DeviceModel::iPhone14Plus;
			}
			else if (modelString == "iPhone15,2")
			{
				return DeviceModel::iPhone14Pro;
			}
			else if (modelString == "iPhone15,3")
			{
				return DeviceModel::iPhone14ProMax;
			}
			else if (modelString == "iPhone15,4")
			{
				return DeviceModel::iPhone15;
			}
			else if (modelString == "iPhone15,5")
			{
				return DeviceModel::iPhone15Plus;
			}
			else if (modelString == "iPhone16,1")
			{
				return DeviceModel::iPhone15Pro;
			}
			else if (modelString == "iPhone16,2")
			{
				return DeviceModel::iPhone15ProMax;
			}
			else if (modelString == "iPhone17,3")
			{
				return DeviceModel::iPhone16;
			}
			else if (modelString == "iPhone17,4")
			{
				return DeviceModel::iPhone16Plus;
			}
			else if (modelString == "iPhone17,1")
			{
				return DeviceModel::iPhone16Pro;
			}
			else if (modelString == "iPhone17,2")
			{
				return DeviceModel::iPhone16ProMax;
			}

			AssertMessage(false, "Unsupported iPhone device {} detected!", modelString);
			return DeviceModel::UnknowniPhone;
		}
		else if (modelString.GetView().StartsWith("iPad"))
		{
			if (modelString == "iPad2,5" || modelString == "iPad2,6" || modelString == "iPad2,7")
			{
				return DeviceModel::iPadMini;
			}
			else if (modelString == "iPad4,4" || modelString == "iPad4,5" || modelString == "iPad4,6")
			{
				return DeviceModel::iPadMini2;
			}
			else if (modelString == "iPad4,7" || modelString == "iPad4,8" || modelString == "iPad4,9")
			{
				return DeviceModel::iPadMini3;
			}
			else if (modelString == "iPad5,1" || modelString == "iPad5,2")
			{
				return DeviceModel::iPadMini4;
			}
			else if (modelString == "iPad11,1" || modelString == "iPad11,2")
			{
				return DeviceModel::iPadMini5;
			}
			else if (modelString == "iPad14,1" || modelString == "iPad14,2")
			{
				return DeviceModel::iPadMini6;
			}
			else if (modelString == "iPad2,1" || modelString == "iPad2,2" || modelString == "iPad2,3" || modelString == "iPad2,4")
			{
				return DeviceModel::iPad2;
			}
			else if (modelString == "iPad3,1" || modelString == "iPad3,2" || modelString == "iPad3,3")
			{
				return DeviceModel::iPad3;
			}
			else if (modelString == "iPad3,4" || modelString == "iPad3,5" || modelString == "iPad3,6")
			{
				return DeviceModel::iPad4;
			}
			else if (modelString == "iPad4,1" || modelString == "iPad4,2" || modelString == "iPad4,3")
			{
				return DeviceModel::iPadAir;
			}
			else if (modelString == "iPad5,3" || modelString == "iPad5,4")
			{
				return DeviceModel::iPadAir2;
			}
			else if (modelString == "iPad6,3" || modelString == "iPad6,4")
			{
				return DeviceModel::iPadPro_9_7;
			}
			else if (modelString == "iPad6,11" || modelString == "iPad6,12")
			{
				return DeviceModel::iPad5;
			}
			else if (modelString == "iPad7,5" || modelString == "iPad7,6")
			{
				return DeviceModel::iPad6;
			}
			else if (modelString == "iPad7,11" || modelString == "iPad7,12")
			{
				return DeviceModel::iPad7;
			}
			else if (modelString == "iPad11,6" || modelString == "iPad11,7")
			{
				return DeviceModel::iPad8;
			}
			else if (modelString == "iPad12,1" || modelString == "iPad12,2")
			{
				return DeviceModel::iPad9;
			}
			else if (modelString == "iPad13,18" || modelString == "iPad13,19")
			{
				return DeviceModel::iPad10;
			}
			else if (modelString == "iPad7,3" || modelString == "iPad7,4")
			{
				return DeviceModel::iPadPro_10_5;
			}
			else if (modelString == "iPad11,3" || modelString == "iPad11,4")
			{
				return DeviceModel::iPadAir3;
			}
			else if (modelString == "iPad13,1" || modelString == "iPad13,2")
			{
				return DeviceModel::iPadAir4;
			}
			else if (modelString == "iPad13,16" || modelString == "iPad13,17")
			{
				return DeviceModel::iPadAir5;
			}
			else if (modelString == "iPad8,1" || modelString == "iPad8,2" || modelString == "iPad8,3" || modelString == "iPad8,4")
			{
				return DeviceModel::iPadPro_11;
			}
			else if (modelString == "iPad8,9" || modelString == "iPad8,10")
			{
				return DeviceModel::iPadPro_11_2nd;
			}
			else if (modelString == "iPad13,4" || modelString == "iPad13,5" || modelString == "iPad13,6" || modelString == "iPad13,7")
			{
				return DeviceModel::iPadPro_11_3rd;
			}
			else if (modelString == "iPad14,3")
			{
				return DeviceModel::iPadPro_11_4th;
			}
			else if (modelString == "iPad6,7" || modelString == "iPad6,8")
			{
				return DeviceModel::iPadPro_12_9;
			}
			else if (modelString == "iPad7,1" || modelString == "iPad7,2")
			{
				return DeviceModel::iPadPro_12_9_2nd;
			}
			else if (modelString == "iPad8,5" || modelString == "iPad8,6" || modelString == "iPad8,7" || modelString == "iPad8,8")
			{
				return DeviceModel::iPadPro_12_9_3rd;
			}
			else if (modelString == "iPad8,11" || modelString == "iPad8,12")
			{
				return DeviceModel::iPadPro_12_9_4th;
			}
			else if (modelString == "iPad13,8" || modelString == "iPad13,9" || modelString == "iPad13,10" || modelString == "iPad13,11")
			{
				return DeviceModel::iPadPro_12_9_5th;
			}
			else if (modelString == "iPad14,5" || modelString == "iPad14,6")
			{
				return DeviceModel::iPadPro_12_9_6th;
			}

			AssertMessage(false, "Unsupported iPad device {} detected!", modelString);
			return DeviceModel::UnknowniPad;
		}
		else if (modelString.GetView().StartsWith("Mac"))
		{
			if (modelString.GetView().StartsWith("MacBookAir"))
			{
				if (modelString == "MacBookAir5,1")
				{
					return DeviceModel::MacBookAir11_2012;
				}
				else if (modelString == "MacBookAir5,2")
				{
					return DeviceModel::MacBookAir13_2012;
				}
				else if (modelString == "MacBookAir6,1")
				{
					return DeviceModel::MacBookAir11_2013;
				}
				else if (modelString == "MacBookAir6,2")
				{
					return DeviceModel::MacBookAir13_2013;
				}
				else if (modelString == "MacBookAir7,1")
				{
					return DeviceModel::MacBookAir11_2015;
				}
				else if (modelString == "MacBookAir7,2")
				{
					return DeviceModel::MacBookAir13_2015;
				}
				else if (modelString == "MacBookAir8,1")
				{
					return DeviceModel::MacBookAir13_2018;
				}
				else if (modelString == "MacBookAir8,2")
				{
					return DeviceModel::MacBookAir13_2019;
				}
				else if (modelString == "MacBookAir9,1")
				{
					return DeviceModel::MacBookAir13_2020;
				}
				else if (modelString == "MacBookAir10,1")
				{
					return DeviceModel::MacBookAir13_M1_2020;
				}

				AssertMessage(false, "Unsupported MacBook Air device {} detected!", modelString);
				return DeviceModel::UnknownMacBookAir;
			}
			else if (modelString.GetView().StartsWith("MacBookPro"))
			{
				if (modelString == "MacBookPro9,1")
				{
					return DeviceModel::MacBookPro15_2012;
				}
				else if (modelString == "MacBookPro9,2")
				{
					return DeviceModel::MacBookPro13_2012;
				}
				else if (modelString == "MacBookPro10,1")
				{
					return DeviceModel::MacBookPro15_2013;
				}
				else if (modelString == "MacBookPro10,2")
				{
					return DeviceModel::MacBookPro13_2013;
				}
				else if (modelString == "MacBookPro11,1")
				{
					return DeviceModel::MacBookPro13_2014;
				}
				else if (modelString == "MacBookPro11,2" || modelString == "MacBookPro11,3")
				{
					return DeviceModel::MacBookPro15_2014;
				}
				else if (modelString == "MacBookPro11,4" || modelString == "MacBookPro11,5")
				{
					return DeviceModel::MacBookPro15_2015;
				}
				else if (modelString == "MacBookPro12,1")
				{
					return DeviceModel::MacBookPro13_2015;
				}
				else if (modelString == "MacBookPro13,1" || modelString == "MacBookPro13,2")
				{
					return DeviceModel::MacBookPro13_2016;
				}
				else if (modelString == "MacBookPro13,3")
				{
					return DeviceModel::MacBookPro15_2016;
				}
				else if (modelString == "MacBookPro14,1" || modelString == "MacBookPro14,2")
				{
					return DeviceModel::MacBookPro13_2017;
				}
				else if (modelString == "MacBookPro14,3")
				{
					return DeviceModel::MacBookPro15_2017;
				}
				else if (modelString == "MacBookPro15,1" || modelString == "MacBookPro15,3")
				{
					return DeviceModel::MacBookPro15_2018;
				}
				else if (modelString == "MacBookPro15,2" || modelString == "MacBookPro15,4")
				{
					return DeviceModel::MacBookPro13_2018;
				}
				else if (modelString == "MacBookPro16,1" || modelString == "MacBookPro16,4")
				{
					return DeviceModel::MacBookPro16_2019;
				}
				else if (modelString == "MacBookPro16,2" || modelString == "MacBookPro16,3")
				{
					return DeviceModel::MacBookPro13_2020;
				}
				else if (modelString == "MacBookPro17,1")
				{
					return DeviceModel::MacBookPro13_M1_2020;
				}
				else if (modelString == "MacBookPro18,1" || modelString == "MacBookPro18,2")
				{
					return DeviceModel::MacBookPro16_2021;
				}
				else if (modelString == "MacBookPro18,3" || modelString == "MacBookPro18,4")
				{
					return DeviceModel::MacBookPro14_2021;
				}

				AssertMessage(false, "Unsupported MacBook Pro device {} detected!", modelString);
				return DeviceModel::UnknownMacBookPro;
			}
			else if (modelString.GetView().StartsWith("MacBook"))
			{
				if (modelString == "MacBook8,1")
				{
					return DeviceModel::MacBook12_2015;
				}
				else if (modelString == "MacBook9,1")
				{
					return DeviceModel::MacBook12_2016;
				}
				else if (modelString == "MacBook10,1")
				{
					return DeviceModel::MacBook12_2017;
				}

				AssertMessage(false, "Unsupported MacBook device {} detected!", modelString);
				return DeviceModel::UnknownMacBook;
			}
			else if (modelString == "Mac13,1")
			{
				return DeviceModel::MacStudio_M1Max;
			}
			else if (modelString == "Mac13,2")
			{
				return DeviceModel::MacStudio_M1Ultra;
			}
			else if (modelString == "Mac14,13")
			{
				return DeviceModel::MacStudio_M2Max;
			}
			else if (modelString == "Mac14,13")
			{
				return DeviceModel::MacStudio_M2Ultra;
			}
			else if (modelString == "Mac14,2")
			{
				return DeviceModel::MacBookAir13_M2;
			}
			else if (modelString == "Mac14,7")
			{
				return DeviceModel::MacBookPro13_M2;
			}
			if (modelString.GetView().StartsWith("Macmini"))
			{
				if (modelString == "Macmini9,1")
				{
					return DeviceModel::MacMini_2020;
				}
				else if (modelString == "Macmini8,1")
				{
					return DeviceModel::MacMini_2018;
				}
				else if (modelString == "Macmini7,1")
				{
					return DeviceModel::MacMini_2014;
				}
				else if (modelString == "Macmini6,2")
				{
					return DeviceModel::MacMini_2012;
				}
				else if (modelString == "Macmini6,1")
				{
					return DeviceModel::MacMini_5_2012;
				}

				AssertMessage(false, "Unsupported Macmini device {} detected!", modelString);
				return DeviceModel::UnknownMacMini;
			}

			AssertMessage(false, "Unsupported Mac device {} detected!", modelString);
			return DeviceModel::UnknownMac;
		}
		else if (modelString.GetView().StartsWith("iMac"))
		{
			if (modelString == "iMac13,1")
			{
				return DeviceModel::iMac21_5_2012;
			}
			else if (modelString == "iMac13,2")
			{
				return DeviceModel::iMac27_2012;
			}
			else if (modelString == "iMac14,1")
			{
				return DeviceModel::iMac21_5_2013;
			}
			else if (modelString == "iMac14,2")
			{
				return DeviceModel::iMac27_2013;
			}
			else if (modelString == "iMac14,4")
			{
				return DeviceModel::iMac21_5_2014;
			}
			else if (modelString == "iMac15,1")
			{
				return DeviceModel::iMac27_2014;
			}
			else if (modelString == "iMac16,1" || modelString == "iMac16,2")
			{
				return DeviceModel::iMac21_5_2015;
			}
			else if (modelString == "iMac17,1")
			{
				return DeviceModel::iMac27_2015;
			}
			else if (modelString == "iMac18,1" || modelString == "iMac18,2")
			{
				return DeviceModel::iMac21_5_2017;
			}
			else if (modelString == "iMac18,3")
			{
				return DeviceModel::iMac27_2017;
			}
			else if (modelString == "iMacPro1,1")
			{
				return DeviceModel::iMacPro27_2017;
			}
			else if (modelString == "iMac19,1")
			{
				return DeviceModel::iMac27_2019;
			}
			else if (modelString == "iMac19,2")
			{
				return DeviceModel::iMac21_5_2019;
			}
			else if (modelString == "iMac20,1" || modelString == "iMac20,2")
			{
				return DeviceModel::iMac27_2020;
			}
			else if (modelString == "iMac21,1" || modelString == "iMac21,2")
			{
				return DeviceModel::iMac24_2021;
			}

			AssertMessage(false, "Unsupported iMac device {} detected!", modelString);
			return DeviceModel::UnknowniMac;
		}
		else if (modelString.GetView().StartsWith("iPod"))
		{
			if (modelString == "iPod5,1")
			{
				return DeviceModel::iPodTouch5;
			}
			else if (modelString == "iPod7,1")
			{
				return DeviceModel::iPodTouch6;
			}
			else if (modelString == "iPod9,1")
			{
				return DeviceModel::iPodTouch7;
			}

			AssertMessage(false, "Unsupported iPod device {} detected!", modelString);
			return DeviceModel::UnknowniPod;
		}

		Assert(false, "Unsupported Apple Device detected, provide screen metrics!");
		return DeviceModel::UnknownApple;
#elif PLATFORM_WINDOWS
		return DeviceModel::UnknownWindows;
#elif PLATFORM_ANDROID
		return DeviceModel::UnknownAndroid;
#elif PLATFORM_WEB
		return DeviceModel::UnknownWeb;
#elif PLATFORM_LINUX
		return DeviceModel::UnknownLinux;
#else
#error "Not implemented for platform!"
#endif
	}

	[[nodiscard]] inline ConstStringView GetDeviceModelFriendlyName()
	{
		switch (Platform::GetDeviceModel())
		{
			case Platform::DeviceModel::iPhone4S:
				return "iPhone 4S";
			case Platform::DeviceModel::iPhone5:
				return "iPhone 5";
			case Platform::DeviceModel::iPhone5C:
				return "iPhone 5C";
			case Platform::DeviceModel::iPhone5S:
				return "iPhone 5S";
			case Platform::DeviceModel::iPhoneSE:
				return "iPhone SE";
			case Platform::DeviceModel::iPhoneSE2:
				return "iPhone SE 2";
			case Platform::DeviceModel::iPhoneSE3:
				return "iPhone SE 3";
			case Platform::DeviceModel::iPodTouch5:
				return "iPad Touch 5th generation";
			case Platform::DeviceModel::iPodTouch6:
				return "iPad Touch 6th generation";
			case Platform::DeviceModel::iPodTouch7:
				return "iPad Touch 7th generation";
			case Platform::DeviceModel::iPhone6:
				return "iPhone 6";
			case Platform::DeviceModel::iPhone6S:
				return "iPhone 6S";
			case Platform::DeviceModel::iPhone7:
				return "iPhone 7";
			case Platform::DeviceModel::iPhone8:
				return "iPhone 8";
			case Platform::DeviceModel::iPhone12Mini:
				return "iPhone 12 Mini";
			case Platform::DeviceModel::iPhone13Mini:
				return "iPhone 13 Mini";
			case Platform::DeviceModel::iPhone6Plus:
				return "iPhone 6 Plus";
			case Platform::DeviceModel::iPhone6SPlus:
				return "iPhone 6S Plus";
			case Platform::DeviceModel::iPhone7Plus:
				return "iPhone 7 Plus";
			case Platform::DeviceModel::iPhone8Plus:
				return "iPhone 8 Plus";
			case Platform::DeviceModel::iPhoneX:
				return "iPhone X";
			case Platform::DeviceModel::iPhoneXS:
				return "iPhone XS";
			case Platform::DeviceModel::iPhone11Pro:
				return "iPhone 11 Pro";
			case Platform::DeviceModel::iPhoneXR:
				return "iPhone XR";
			case Platform::DeviceModel::iPhone11:
				return "iPhone 11";
			case Platform::DeviceModel::iPhone12:
				return "iPhone 12";
			case Platform::DeviceModel::iPhone12Pro:
				return "iPhone 12 Pro";
			case Platform::DeviceModel::iPhone13:
				return "iPhone 13";
			case Platform::DeviceModel::iPhone13Pro:
				return "iPhone 13 Pro";
			case Platform::DeviceModel::iPhoneXSMax:
				return "iPhone XS Max";
			case Platform::DeviceModel::iPhone11ProMax:
				return "iPhone 11 Pro Max";
			case Platform::DeviceModel::iPhone12ProMax:
				return "iPhone 12 Pro Max";
			case Platform::DeviceModel::iPhone13ProMax:
				return "iPhone 13 Pro Max";
			case Platform::DeviceModel::iPhone14:
				return "iPhone 14";
			case Platform::DeviceModel::iPhone14Plus:
				return "iPhone 14 Plus";
			case Platform::DeviceModel::iPhone14Pro:
				return "iPhone 14 Pro";
			case Platform::DeviceModel::iPhone14ProMax:
				return "iPhone 14 Pro Max";
			case Platform::DeviceModel::iPhone15:
				return "iPhone 15";
			case Platform::DeviceModel::iPhone15Plus:
				return "iPhone 15 Plus";
			case Platform::DeviceModel::iPhone15Pro:
				return "iPhone 15 Pro";
			case Platform::DeviceModel::iPhone15ProMax:
				return "iPhone 15 Pro Max";
			case Platform::DeviceModel::iPhone16:
				return "iPhone 16";
			case Platform::DeviceModel::iPhone16Plus:
				return "iPhone 16 Plus";
			case Platform::DeviceModel::iPhone16Pro:
				return "iPhone 16 Pro";
			case Platform::DeviceModel::iPhone16ProMax:
				return "iPhone 16 Pro Max";

			case Platform::DeviceModel::iPadMini:
				return "iPad Mini";
			case Platform::DeviceModel::iPadMini2:
				return "iPad Mini 2nd Generation";
			case Platform::DeviceModel::iPadMini3:
				return "iPad Mini 3rd Generation";
			case Platform::DeviceModel::iPadMini4:
				return "iPad Mini 4th Generation";
			case Platform::DeviceModel::iPadMini5:
				return "iPad Mini 5th Generation";
			case Platform::DeviceModel::iPadMini6:
				return "iPad Mini 6th Generation";
			case Platform::DeviceModel::iPad2:
				return "iPad 2nd Generation";
			case Platform::DeviceModel::iPad3:
				return "iPad 3rd Generation";
			case Platform::DeviceModel::iPad4:
				return "iPad 4th Generation";
			case Platform::DeviceModel::iPadAir:
				return "iPad Air";
			case Platform::DeviceModel::iPadAir2:
				return "iPad Air 2nd Generation";
			case Platform::DeviceModel::iPadPro_9_7:
				return "iPad Pro 9.7 inch";
			case Platform::DeviceModel::iPad5:
				return "iPad 5th Generation";
			case Platform::DeviceModel::iPad6:
				return "iPad 6th Generation";
			case Platform::DeviceModel::iPad7:
				return "iPad 7th Generation";
			case Platform::DeviceModel::iPad8:
				return "iPad 8th Generation";
			case Platform::DeviceModel::iPad9:
				return "iPad 9th Generation";
			case Platform::DeviceModel::iPad10:
				return "iPad 10th Generation";
			case Platform::DeviceModel::iPadPro_10_5:
				return "iPad Pro 10.5 inch";
			case Platform::DeviceModel::iPadAir3:
				return "iPad Air 3rd Generation";
			case Platform::DeviceModel::iPadAir4:
				return "iPad Air 4th Generation";
			case Platform::DeviceModel::iPadAir5:
				return "iPad Air 5th Generation";
			case Platform::DeviceModel::iPadPro_11:
				return "iPad Pro 11 inch";
			case Platform::DeviceModel::iPadPro_11_2nd:
				return "iPad Pro 11 inch 2nd Generation";
			case Platform::DeviceModel::iPadPro_11_3rd:
				return "iPad Pro 11 inch 3rd Generation";
			case Platform::DeviceModel::iPadPro_11_4th:
				return "iPad Pro 11 inch 4th Generation";
			case Platform::DeviceModel::iPadPro_12_9:
				return "iPad Pro 12.9 inch 1st Generation";
			case Platform::DeviceModel::iPadPro_12_9_2nd:
				return "iPad Pro 12.9 inch 2nd Generation";
			case Platform::DeviceModel::iPadPro_12_9_3rd:
				return "iPad Pro 12.9 inch 3rd Generation";
			case Platform::DeviceModel::iPadPro_12_9_4th:
				return "iPad Pro 12.9 inch 4th Generation";
			case Platform::DeviceModel::iPadPro_12_9_5th:
				return "iPad Pro 12.9 inch 5th Generation";
			case Platform::DeviceModel::iPadPro_12_9_6th:
				return "iPad Pro 12.9 inch 6th Generation";

			case Platform::DeviceModel::MacBookAir11_2012:
				return "MacBook Air 11 inch 2012";
			case Platform::DeviceModel::MacBookAir11_2013:
				return "MacBook Air 11 inch 2013";
			case Platform::DeviceModel::MacBookAir11_2015:
				return "MacBook Air 11 inch 2015";

			case Platform::DeviceModel::MacBookAir13_2012:
				return "MacBook Air 13 inch 2012";
			case Platform::DeviceModel::MacBookAir13_2013:
				return "MacBook Air 13 inch 2013";
			case Platform::DeviceModel::MacBookAir13_2015:
				return "MacBook Air 13 inch 2015";
			case Platform::DeviceModel::MacBookAir13_2018:
				return "MacBook Air 13 inch 2018";
			case Platform::DeviceModel::MacBookAir13_2019:
				return "MacBook Air 13 inch 2019";
			case Platform::DeviceModel::MacBookAir13_2020:
				return "MacBook Air 13 inch 2020";
			case Platform::DeviceModel::MacBookAir13_M1_2020:
				return "MacBook Air 13 inch M1 2020";

			case Platform::DeviceModel::MacBookPro13_2012:
				return "MacBook Pro 13 inch 2012";
			case Platform::DeviceModel::MacBookPro13_2013:
				return "MacBook Pro 13 inch 2013";
			case Platform::DeviceModel::MacBookPro13_2014:
				return "MacBook Pro 13 inch 2014";
			case Platform::DeviceModel::MacBookPro13_2015:
				return "MacBook Pro 13 inch 2015";
			case Platform::DeviceModel::MacBookPro13_2016:
				return "MacBook Pro 13 inch 2016";
			case Platform::DeviceModel::MacBookPro13_2017:
				return "MacBook Pro 13 inch 2017";
			case Platform::DeviceModel::MacBookPro13_2018:
				return "MacBook Pro 13 inch 2018";
			case Platform::DeviceModel::MacBookPro13_2020:
				return "MacBook Pro 13 inch 2020";
			case Platform::DeviceModel::MacBookPro13_M1_2020:
				return "MacBook Pro 13 inch M1 2020";
			case Platform::DeviceModel::MacBookPro13_M2:
				return "MacBook Pro 13 inch M2 2022";

			case Platform::DeviceModel::MacBookAir13_M2:
				return "MacBook Air 13 inch M2 2022";

			case Platform::DeviceModel::MacBookPro15_2012:
				return "MacBook Pro 15 inch 2012";
			case Platform::DeviceModel::MacBookPro15_2013:
				return "MacBook Pro 15 inch 2013";
			case Platform::DeviceModel::MacBookPro15_2014:
				return "MacBook Pro 15 inch 2014";
			case Platform::DeviceModel::MacBookPro15_2015:
				return "MacBook Pro 15 inch 2015";
			case Platform::DeviceModel::MacBookPro15_2016:
				return "MacBook Pro 15 inch 2016";
			case Platform::DeviceModel::MacBookPro15_2017:
				return "MacBook Pro 15 inch 2017";
			case Platform::DeviceModel::MacBookPro15_2018:
				return "MacBook Pro 15 inch 2018";

			case Platform::DeviceModel::MacBookPro14_2021:
				return "MacBook Pro 14 inch 2021";

			case Platform::DeviceModel::MacBookPro16_2019:
				return "MacBook Pro 16 inch 2019";
			case Platform::DeviceModel::MacBookPro16_2021:
				return "MacBook Pro 16 inch 2021";

			case Platform::DeviceModel::MacBook12_2015:
				return "MacBook Pro 12 inch 2015";
			case Platform::DeviceModel::MacBook12_2016:
				return "MacBook Pro 12 inch 2016";
			case Platform::DeviceModel::MacBook12_2017:
				return "MacBook Pro 12 inch 2017";

			case Platform::DeviceModel::MacMini_5_2012:
				return "Mac mini \"Core i5\" 2012";
			case Platform::DeviceModel::MacMini_2012:
				return "Mac mini \"Core i7\" 2012";
			case Platform::DeviceModel::MacMini_2014:
				return "Mac mini 2014";
			case Platform::DeviceModel::MacMini_2018:
				return "Mac mini 2018";
			case Platform::DeviceModel::MacMini_2020:
				return "Mac mini 2020";

			case Platform::DeviceModel::MacStudio_M1Max:
				return "Mac Studio M1 Max";
			case Platform::DeviceModel::MacStudio_M1Ultra:
				return "Mac Studio M1 Ultra";
			case Platform::DeviceModel::MacStudio_M2Max:
				return "Mac Studio M2 Max";
			case Platform::DeviceModel::MacStudio_M2Ultra:
				return "Mac Studio M2 Ultra";

			case Platform::DeviceModel::iMac21_5_2012:
				return "iMac 21.5 inch 2012";
			case Platform::DeviceModel::iMac21_5_2013:
				return "iMac 21.5 inch 2013";
			case Platform::DeviceModel::iMac21_5_2014:
				return "iMac 21.5 inch 2014";
			case Platform::DeviceModel::iMac21_5_2015:
				return "iMac 21.5 inch 2015";
			case Platform::DeviceModel::iMac21_5_2017:
				return "iMac 21.5 inch 2017";
			case Platform::DeviceModel::iMac21_5_2019:
				return "iMac 21.5 inch 2019";

			case Platform::DeviceModel::iMac27_2012:
				return "iMac 27 inch 2012";
			case Platform::DeviceModel::iMac27_2013:
				return "iMac 27 inch 2013";
			case Platform::DeviceModel::iMac27_2014:
				return "iMac 27 inch 2014";
			case Platform::DeviceModel::iMac27_2015:
				return "iMac 27 inch 2015";
			case Platform::DeviceModel::iMac27_2017:
				return "iMac 27 inch 2017";
			case Platform::DeviceModel::iMacPro27_2017:
				return "iMac Pro 27 inch 2017";
			case Platform::DeviceModel::iMac27_2019:
				return "iMac 27 inch 2019";
			case Platform::DeviceModel::iMac24_2021:
				return "iMac 24 inch 2021";

			case Platform::DeviceModel::iMac27_2020:
				return "iMac 27 inch 2020";

			case Platform::DeviceModel::UnknowniPad:
				Assert(false, "Unsupported iPad detected, provide data!");
				return "Unknown iPad Model";

			case Platform::DeviceModel::UnknowniPhone:
				Assert(false, "Unsupported iPhone detected, provide data!");
				return "Unknown iPhone Model";

			case Platform::DeviceModel::UnknowniPod:
				Assert(false, "Unsupported iPod detected, provide data!");
				return "Unknown iPod Model";

			case Platform::DeviceModel::UnknownMacBook:
				Assert(false, "Unsupported MacBook detected, provide data!");
				return "Unknown MacBook Model";

			case Platform::DeviceModel::UnknownMacBookAir:
				Assert(false, "Unsupported MacBook Air detected, provide data!");
				return "Unknown MacBook Air Model";

			case Platform::DeviceModel::UnknownMacBookPro:
				Assert(false, "Unsupported MacBook Pro detected, provide data!");
				return "Unknown MacBook Pro Model";

			case Platform::DeviceModel::UnknownMac:
				Assert(false, "Unsupported Mac detected, provide data!");
				return "Unknown Mac Model";

			case Platform::DeviceModel::UnknownMacMini:
				Assert(false, "Unsupported Macmini detected, provide data!");
				return "Unknown Macmini Model";

			case Platform::DeviceModel::UnknowniMac:
				Assert(false, "Unsupported iMac detected, provide data!");
				return "Unknown iMac Model";

			case Platform::DeviceModel::VisionPro:
				return "Vision Pro";

			case Platform::DeviceModel::UnknownVision:
				Assert(false, "Unsupported Vision device detected, provide data!");
				return "Unknown Vision Device";

			case Platform::DeviceModel::UnknownApple:
				Assert(false, "Unsupported Apple device detected, provide data!");
				return "Unknown Apple Device";

			case Platform::DeviceModel::UnknownWindows:
				return "Unknown Windows Device";

			case Platform::DeviceModel::UnknownAndroid:
				return "Unknown Android Device";

			case Platform::DeviceModel::UnknownWeb:
				return "Unknown Web Device";

			case Platform::DeviceModel::UnknownLinux:
				return "Unknown Linux Device";
		}

		return "Unknown Device";
	}
}

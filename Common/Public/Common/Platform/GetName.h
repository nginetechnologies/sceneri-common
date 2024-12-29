#pragma once

#include <Common/Platform/Type.h>
#include <Common/Memory/Containers/StringView.h>

namespace ngine::Platform
{
	namespace Internal
	{
		static inline constexpr ConstNativeStringView WindowsName = MAKE_NATIVE_LITERAL("Windows");
		static inline constexpr ConstNativeStringView MacOSName = MAKE_NATIVE_LITERAL("macOS");
		static inline constexpr ConstNativeStringView iOSName = MAKE_NATIVE_LITERAL("iOS");
		static inline constexpr ConstNativeStringView MacCatalystName = MAKE_NATIVE_LITERAL("macCatalyst");
		static inline constexpr ConstNativeStringView VisionOSName = MAKE_NATIVE_LITERAL("visionOS");
		static inline constexpr ConstNativeStringView AndroidName = MAKE_NATIVE_LITERAL("Android");
		static inline constexpr ConstNativeStringView WebName = MAKE_NATIVE_LITERAL("Web");
		static inline constexpr ConstNativeStringView LinuxName = MAKE_NATIVE_LITERAL("Linux");
	}

	inline constexpr ConstNativeStringView GetName(const Type platform)
	{
		switch (platform)
		{
			case Type::Windows:
				return Internal::WindowsName;
			case Type::macOS:
				return Internal::MacOSName;
			case Type::iOS:
				return Internal::iOSName;
			case Type::macCatalyst:
				return Internal::MacCatalystName;
			case Type::visionOS:
				return Internal::VisionOSName;
			case Type::Android:
				return Internal::AndroidName;
			case Type::Web:
				return Internal::WebName;
			case Type::Linux:
				return Internal::LinuxName;
			case Type::All:
			case Type::Apple:
				// case Type::Count:
				ExpectUnreachable();
		}

		ExpectUnreachable();
	}

	inline constexpr Type GetFromName(const ConstNativeStringView name)
	{
		if (name == Internal::WindowsName)
		{
			return Type::Windows;
		}
		else if (name == Internal::MacOSName)
		{
			return Type::macOS;
		}
		else if (name == Internal::iOSName)
		{
			return Type::iOS;
		}
		else if (name == Internal::MacCatalystName)
		{
			return Type::macCatalyst;
		}
		else if (name == Internal::VisionOSName)
		{
			return Type::visionOS;
		}
		else if (name == Internal::AndroidName)
		{
			return Type::Android;
		}
		else if (name == Internal::WebName)
		{
			return Type::Web;
		}
		else if (name == Internal::LinuxName)
		{
			return Type::Linux;
		}

		ExpectUnreachable();
	}
}

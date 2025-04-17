#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/EnumFlagOperators.h>
#include <Common/Math/Log2.h>
#include <Common/Platform/Pure.h>

namespace ngine::Platform
{
	enum class Type : uint8
	{
		Windows = 1 << 0,
		macOS = 1 << 1,
		iOS = 1 << 2,
		macCatalyst = 1 << 3,
		visionOS = 1 << 4,
		Android = 1 << 5,
		Web = 1 << 6,
		Linux = 1 << 7,
		Count = Math::Log2(Linux) + 1,
		Apple = macOS | iOS | macCatalyst | visionOS,
		All = Windows | macOS | iOS | macCatalyst | visionOS | Android | Web | Linux
	};

	ENUM_FLAG_OPERATORS(Type);

#if PLATFORM_WINDOWS
	inline static constexpr Type Current = Type::Windows;
#elif PLATFORM_APPLE_MACCATALYST
	inline static constexpr Type Current = Type::macCatalyst;
#elif PLATFORM_APPLE_IOS
	inline static constexpr Type Current = Type::iOS;
#elif PLATFORM_APPLE_VISIONOS
	inline static constexpr Type Current = Type::visionOS;
#elif PLATFORM_APPLE_MACOS
	inline static constexpr Type Current = Type::macOS;
#elif PLATFORM_ANDROID
	inline static constexpr Type Current = Type::Android;
#elif PLATFORM_WEB
	inline static constexpr Type Current = Type::Web;
#elif PLATFORM_LINUX
	inline static constexpr Type Current = Type::Linux;
#else
#error "Unknown platform"
#endif

	//! Checks the effective platform
	//! Generally returns the same result as Type::Current, except for Web where the underlying platform can differ
	[[nodiscard]] PURE_STATICS Type GetEffectiveType();

	enum class Class : uint8
	{
		//! Laptops and computers (Windows, Linux & macOS)
		Desktop,
		//! Phones and tablets (Android, iOS & Linux)
		//! Note: Devices such as SteamDeck will classify as mobile despite running traditionally desktop-y OS
		Mobile,
		//! Immersive XR devices (visionOS)
		Spatial
	};
	[[nodiscard]] PURE_STATICS Class GetClass();
}

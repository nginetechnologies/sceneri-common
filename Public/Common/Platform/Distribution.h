#pragma once

#include <Common/Platform/StaticUnreachable.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Memory/Containers/StringView.h>

namespace ngine::Platform
{
	enum class Distribution : uint8
	{
		Local,
		Steam,
		GooglePlay,
		AppStore,
		Docker
	};

	[[nodiscard]] inline static constexpr Distribution GetDistribution(const ConstStringView distribution)
	{
		if (distribution == "local")
		{
			return Distribution::Local;
		}
		else if (distribution == "steam")
		{
			return Distribution::Steam;
		}
		else if (distribution == "googleplay")
		{
			return Distribution::GooglePlay;
		}
		else if (distribution == "appstore")
		{
			return Distribution::AppStore;
		}
		else if (distribution == "docker")
		{
			return Distribution::Docker;
		}
		else
		{
			return Distribution::Local;
		}
	}

	inline static constexpr Distribution TargetDistribution = GetDistribution(TARGET_DISTRIBUTION);
}

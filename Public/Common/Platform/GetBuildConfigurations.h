#pragma once

#include <Common/Memory/Containers/StringView.h>

namespace ngine::Platform
{
	struct BuildConfigurations
	{
		using Container = Array<ConstStringView, 3>;

		Container m_configurations;
		uint8 m_count = 0u;

		[[nodiscard]] FORCE_INLINE constexpr typename Container::const_iterator begin() const
		{
			return m_configurations.begin();
		}
		[[nodiscard]] FORCE_INLINE constexpr typename Container::const_iterator end() const
		{
			return m_configurations.begin() + m_count;
		}
	};

	[[nodiscard]] inline constexpr BuildConfigurations GetBuildConfigurations()
	{
		BuildConfigurations buildConfigurations;

		ConstStringView buildConfigurationsString = PLATFORM_CONFIGURATION_TYPES;
		for (uint32 index = buildConfigurationsString.FindFirstOf(',', 0); index != StringView::InvalidPosition;)
		{
			buildConfigurations.m_configurations[buildConfigurations.m_count] = buildConfigurationsString.GetSubstring(0, index);
			buildConfigurations.m_count++;

			buildConfigurationsString = buildConfigurationsString.GetSubstring(index + 1, buildConfigurationsString.GetSize() - index - 1);
			index = buildConfigurationsString.FindFirstOf(',', 0);
		}

		buildConfigurations.m_configurations[buildConfigurations.m_count] = buildConfigurationsString;
		buildConfigurations.m_count++;

		return buildConfigurations;
	}
}

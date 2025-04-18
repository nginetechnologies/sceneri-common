#pragma once

#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Containers/FlatVector.h>

namespace ngine::Platform
{
	struct Types
	{
		struct Entry
		{
			ConstStringView name;
			bool isActive;
		};

		using Container = FlatVector<Entry, 20>;
		Container m_configurations;

		[[nodiscard]] FORCE_INLINE constexpr typename Container::const_iterator begin() const
		{
			return m_configurations.begin();
		}
		[[nodiscard]] FORCE_INLINE constexpr typename Container::const_iterator end() const
		{
			return m_configurations.end();
		}
	};

	[[nodiscard]] inline Types GetTypes()
	{
		Types buildConfigurations;

		auto emplaceEntry = [&buildConfigurations](const ConstStringView platformEntry)
		{
			const uint32 delimiterIndex = platformEntry.FindFirstOf('=', 0) + 1;
			buildConfigurations.m_configurations.EmplaceBack(Types::Entry{
				platformEntry.GetSubstring(0, delimiterIndex - 1),
				platformEntry.GetSubstring(delimiterIndex, platformEntry.GetSize() - delimiterIndex).ToInteger() != 0
			});
		};

		ConstStringView typesString = PLATFORM_TYPES;
		for (uint32 index = typesString.FindFirstOf(',', 0); index != StringView::InvalidPosition;)
		{
			const ConstStringView platformEntry = typesString.GetSubstring(0, index);
			emplaceEntry(platformEntry);

			typesString = typesString.GetSubstring(index + 1, typesString.GetSize() - index - 1);
			index = typesString.FindFirstOf(',', 0);
		}

		emplaceEntry(typesString);

		return buildConfigurations;
	}
}

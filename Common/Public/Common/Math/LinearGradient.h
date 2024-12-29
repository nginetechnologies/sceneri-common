#pragma once

#include <Common/Math/Angle.h>
#include <Common/Math/Ratio.h>
#include <Common/Math/Color.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Memory/Containers/InlineVector.h>

namespace ngine::Math
{
	struct LinearGradient
	{
		struct Color
		{
			Math::Color m_color;
			Math::Ratiof m_stopPoint;
		};
		using ColorContainer = InlineVector<Color, 3>;

		LinearGradient() = default;
		LinearGradient(const Math::Anglef orientation, ColorContainer&& colors)
			: m_orientation(orientation)
			, m_colors(ngine::Forward<ColorContainer>(colors))
		{
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_colors.HasElements();
		}

		bool operator==(const LinearGradient& other)
		{
			auto compare = [](const LinearGradient& left, const LinearGradient& right) -> bool
			{
				if (left.m_colors.GetSize() != right.m_colors.GetSize())
				{
					return false;
				}

				for (uint32 i = 0; i < left.m_colors.GetSize(); i++)
				{
					if (left.m_colors[i].m_color == right.m_colors[i].m_color && left.m_colors[i].m_stopPoint == right.m_colors[i].m_stopPoint)
					{
						return false;
					}
				}

				return true;
			};

			return m_orientation == other.m_orientation && compare(*this, other);
		}

		inline bool operator!=(const LinearGradient& other)
		{
			return !(*this == other);
		}

		Math::Anglef m_orientation;
		ColorContainer m_colors;
	};
}

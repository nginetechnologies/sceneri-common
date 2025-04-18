#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/NumericLimits.h>

namespace ngine::Network
{
	struct Port
	{
		constexpr Port()
			: m_port(Math::NumericLimits<uint16>::Max)
		{
		}
		constexpr Port(const uint16 port)
			: m_port(port)
		{
		}
		[[nodiscard]] bool IsValid() const
		{
			return m_port != Math::NumericLimits<uint16>::Max;
		}
		[[nodiscard]] constexpr uint16 Get() const
		{
			return m_port;
		}

		[[nodiscard]] static constexpr Port Any()
		{
			return Port{0};
		}
		[[nodiscard]] static constexpr Port Default()
		{
			return Port{30159};
		}
	protected:
		uint16 m_port;
	};
}

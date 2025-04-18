#pragma once

#include "Port.h"

#include <Common/IO/ForwardDeclarations/ZeroTerminatedURIView.h>
#include <Common/IO/ForwardDeclarations/URI.h>

namespace ngine::Network
{
	struct IPAddress4
	{
		constexpr IPAddress4()
			: m_address(Math::NumericLimits<uint32>::Max)
		{
		}
		constexpr IPAddress4(const uint32 address)
			: m_address(address)
		{
		}
		IPAddress4(const IO::ConstZeroTerminatedURIView address);

		[[nodiscard]] static constexpr IPAddress4 Any()
		{
			return IPAddress4{0};
		}

		[[nodiscard]] IPAddress4 GetReverseLookupAddress() const;
		[[nodiscard]] IO::URI ToURI() const;

		[[nodiscard]] bool IsValid() const
		{
			return m_address != Math::NumericLimits<uint32>::Max;
		}
		[[nodiscard]] constexpr uint32 Get() const
		{
			return m_address;
		}
	protected:
		uint32 m_address;
	};

	struct IPAddress6
	{
		constexpr IPAddress6()
			: m_address(Math::NumericLimits<uint128>::Max)
		{
		}
		constexpr IPAddress6(const uint128 address)
			: m_address(address)
		{
		}
		IPAddress6(const IO::ConstZeroTerminatedURIView address);

		[[nodiscard]] static constexpr IPAddress6 Any()
		{
			return IPAddress6{uint128(0)};
		}

		[[nodiscard]] IPAddress6 GetReverseLookupAddress() const;
		[[nodiscard]] IO::URI ToURI() const;

		[[nodiscard]] bool IsValid() const
		{
			return m_address != Math::NumericLimits<uint128>::Max;
		}
		[[nodiscard]] constexpr uint128 Get() const
		{
			return m_address;
		}
	protected:
		uint128 m_address;
	};

	struct IPAddress
	{
		enum class Type : uint8
		{
			Invalid,
			IPv4,
			IPv6
		};

		IPAddress()
			: m_type{Type::Invalid}
		{
		}
		constexpr IPAddress(const IPAddress4 address)
			: m_type{Type::IPv4}
			, m_ipv4{address}
		{
		}
		constexpr IPAddress(const IPAddress6 address)
			: m_type{Type::IPv6}
			, m_ipv6{address}
		{
		}
		IPAddress(const IO::ConstZeroTerminatedURIView address);

		[[nodiscard]] static constexpr IPAddress Any()
		{
			return IPAddress4{0};
		}

		[[nodiscard]] IPAddress GetReverseLookupAddress() const;
		[[nodiscard]] IO::URI ToURI() const;

		[[nodiscard]] constexpr bool IsValid() const
		{
			return m_type != Type::Invalid;
		}
		[[nodiscard]] constexpr Type GetType() const
		{
			return m_type;
		}
		[[nodiscard]] constexpr IPAddress4 GetIPv4() const
		{
			return m_type == Type::IPv4 ? m_ipv4 : IPAddress4{};
		}
		[[nodiscard]] constexpr IPAddress6 GetIPv6() const
		{
			return m_type == Type::IPv6 ? m_ipv6 : IPAddress6{};
		}
	protected:
		Type m_type{Type::Invalid};
		union
		{
			IPAddress4 m_ipv4;
			IPAddress6 m_ipv6;
		};
	};

	inline static constexpr IPAddress AnyIPAddress{IPAddress::Any()};

	struct Address
	{
		Address() = default;
		constexpr Address(const IPAddress address, const Port port = Port::Default())
			: m_ipAddress(address)
			, m_port(port)
		{
		}
		Address(const IO::ConstZeroTerminatedURIView address);

		[[nodiscard]] constexpr IPAddress GetIPAddress() const
		{
			return m_ipAddress;
		}
		[[nodiscard]] constexpr IPAddress::Type GetType() const
		{
			return m_ipAddress.GetType();
		}
		[[nodiscard]] constexpr Port GetPort() const
		{
			return m_port;
		}

		[[nodiscard]] IO::URI ToURI() const;
	protected:
		IPAddress m_ipAddress;
		Port m_port{Port::Default()};
	};
}

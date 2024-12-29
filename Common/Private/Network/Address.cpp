#include "Network/Address.h"

#if PLATFORM_POSIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#include <mmsystem.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#endif

#include <Common/IO/URI.h>
#include <Common/IO/Format/URI.h>
#include <Common/Memory/Containers/Format/StringView.h>

namespace ngine::Network
{
	IPAddress4::IPAddress4(const IO::ConstZeroTerminatedURIView address)
	{
		if (address.HasElements())
		{
			// Start by seeing if we can perform a local conversion from address
			if (address.GetSize() < INET_ADDRSTRLEN - 1)
			{
				uint32 ipv4Address;
				if (inet_pton(AF_INET, address, &ipv4Address) == 1)
				{
					m_address = ipv4Address;
					return;
				}
			}

			// Attempt hostname lookup
#if PLATFORM_WEB
			// Disabled for now as we don't expose networking on web
#elif PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_ANDROID
			struct addrinfo* resultList = nullptr;
			if (getaddrinfo(address, NULL, NULL, &resultList) == 0)
			{
				for (addrinfo* result = resultList; result != NULL; result = result->ai_next)
				{
					if (result->ai_family == AF_INET && result->ai_addr != NULL && result->ai_addrlen >= sizeof(struct sockaddr_in))
					{
						PUSH_CLANG_WARNINGS
						DISABLE_CLANG_WARNING("-Wcast-align")
						struct sockaddr_in* sin = (struct sockaddr_in*)result->ai_addr;
						POP_CLANG_WARNINGS
						m_address = sin->sin_addr.s_addr;
						freeaddrinfo(resultList);
						return;
					}
				}

				if (resultList != nullptr)
				{
					freeaddrinfo(resultList);
				}
			}
#elif PLATFORM_POSIX
			struct hostent* hostEntry = gethostbyname(address);
			if (hostEntry != nullptr)
			{
				if (hostEntry->h_addrtype == AF_INET)
				{
					PUSH_CLANG_WARNINGS
					DISABLE_CLANG_WARNING("-Wcast-align")
					m_address = *(uint32*)hostEntry->h_addr_list[0];
					POP_CLANG_WARNINGS
					return;
				}
			}
#else
#error "Not implemented for platform"
#endif
		}
	}

	IPAddress4 IPAddress4::GetReverseLookupAddress() const
	{
#if PLATFORM_WEB
		// Disabled for now as we don't expose networking
		return {};
#elif PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_ANDROID || PLATFORM_LINUX
		struct sockaddr_in sin;

		Memory::Set(&sin, 0, sizeof(struct sockaddr_in));

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = m_address;

		Array<char, INET_ADDRSTRLEN> buffer;
		const int result = getnameinfo((struct sockaddr*)&sin, sizeof(sin), buffer.GetData(), buffer.GetSize(), NULL, 0, NI_NAMEREQD);
		if (result != 0)
		{
			return IPAddress4{};
		}

		uint32 ipv4Address;
		if (inet_pton(AF_INET, buffer.GetData(), &ipv4Address) == 1)
		{
			return IPAddress4{ipv4Address};
		}
		else
		{
			return IPAddress4{};
		}
#else
#error "Not implemented for platform!"
#endif
	}

	IO::URI IPAddress4::ToURI() const
	{
#if PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_POSIX || PLATFORM_WEB || PLATFORM_ANDROID || PLATFORM_LINUX
		in_addr address;
		address.s_addr = m_address;
		Array<char, INET_ADDRSTRLEN> targetBuffer;
		const bool success = inet_ntop(AF_INET, &address, targetBuffer.GetData(), targetBuffer.GetSize()) != nullptr;
		Assert(success);
		if (LIKELY(success))
		{
			return IO::URI(IO::URI::StringType(targetBuffer.GetData(), INET_ADDRSTRLEN));
		}
		else
		{
			return {};
		}
#else
#error "Not implemented for platform"
#endif
	}

	IPAddress6::IPAddress6(const IO::ConstZeroTerminatedURIView address)
	{
		if (address.HasElements())
		{
			// Start by seeing if we can perform a local conversion from address
			uint128 ipv6Address;
			if (inet_pton(AF_INET6, address, &ipv6Address) == 1)
			{
				m_address = ipv6Address;
				return;
			}

			// Attempt hostname lookup
#if PLATFORM_WEB
			// Disabled for now as we don't expose networking on web
#elif PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_ANDROID || PLATFORM_LINUX
			struct addrinfo* resultList = nullptr;
			if (getaddrinfo(address, NULL, NULL, &resultList) == 0)
			{
				for (addrinfo* result = resultList; result != NULL; result = result->ai_next)
				{
					if (result->ai_family == AF_INET6 && result->ai_addr != NULL && result->ai_addrlen >= sizeof(struct sockaddr_in6))
					{
						PUSH_CLANG_WARNINGS
						DISABLE_CLANG_WARNING("-Wcast-align")
						struct sockaddr_in6* sin = (struct sockaddr_in6*)result->ai_addr;
						POP_CLANG_WARNINGS
						static_assert(sizeof(sin->sin6_addr) == sizeof(uint128));
						m_address = reinterpret_cast<const uint128&>(sin->sin6_addr);
						freeaddrinfo(resultList);
						return;
					}
				}

				if (resultList != nullptr)
				{
					freeaddrinfo(resultList);
				}
			}
#elif PLATFORM_POSIX
			struct hostent* hostEntry = gethostbyname(address);
			if (hostEntry != nullptr)
			{
				if (hostEntry->h_addrtype == AF_INET6)
				{
					PUSH_CLANG_WARNINGS
					DISABLE_CLANG_WARNING("-Wcast-align")
					m_address = *(uint128*)hostEntry->h_addr_list[0];
					POP_CLANG_WARNINGS
					return;
				}
			}
#else
#error "Not implemented for platform"
#endif
		}
	}

	IPAddress6 IPAddress6::GetReverseLookupAddress() const
	{
#if PLATFORM_WEB
		// Disabled for now as we don't expose networking
		return {};
#elif PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_ANDROID || PLATFORM_LINUX
		struct sockaddr_in6 sin;

		Memory::Set(&sin, 0, sizeof(struct sockaddr_in6));

		sin.sin6_family = AF_INET6;
		reinterpret_cast<uint128&>(sin.sin6_addr) = m_address;

		Array<char, INET6_ADDRSTRLEN> buffer;
		const int result = getnameinfo((struct sockaddr*)&sin, sizeof(sin), buffer.GetData(), buffer.GetSize(), NULL, 0, NI_NAMEREQD);
		if (result != 0)
		{
			return IPAddress6{};
		}

		uint128 ipv6Address;
		if (inet_pton(AF_INET6, buffer.GetData(), &ipv6Address) == 1)
		{
			return IPAddress6{ipv6Address};
		}
		else
		{
			return IPAddress6{};
		}
#else
#error "Not implemented for platform!"
#endif
	}

	IO::URI IPAddress6::ToURI() const
	{
#if PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_POSIX || PLATFORM_WEB || PLATFORM_ANDROID || PLATFORM_LINUX
		in6_addr address;
		reinterpret_cast<uint128&>(address) = m_address;
		Array<char, INET6_ADDRSTRLEN> targetBuffer;
		const bool success = inet_ntop(AF_INET6, &address, targetBuffer.GetData(), targetBuffer.GetSize()) != nullptr;
		Assert(success);
		if (LIKELY(success))
		{
			return IO::URI(IO::URI::StringType(targetBuffer.GetData(), (IO::URI::SizeType)strlen(targetBuffer.GetData())));
		}
		else
		{
			return {};
		}
#else
#error "Not implemented for platform"
#endif
	}

	IPAddress::IPAddress(const IO::ConstZeroTerminatedURIView address)
	{
		if (address.HasElements())
		{
			// Start by seeing if we can perform a local conversion from address
			if (address.GetSize() < INET_ADDRSTRLEN)
			{
				uint32 ipv4Address;
				if (inet_pton(AF_INET, address, &ipv4Address) == 1)
				{
					*this = IPAddress4{ipv4Address};
					return;
				}
			}
			else
			{
				uint128 ipv6Address;
				if (inet_pton(AF_INET6, address, &ipv6Address) == 1)
				{
					*this = IPAddress6{ipv6Address};
					return;
				}
			}

			// Attempt hostname lookup
#if PLATFORM_WEB
			// Disabled for now as we don't expose networking on web
#elif PLATFORM_APPLE || PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN || PLATFORM_ANDROID || PLATFORM_LINUX
			struct addrinfo hints
			{
				0
			};
			// Only return IPV4 for now
			hints.ai_family = AF_INET;
			struct addrinfo* resultList = nullptr;
			if (getaddrinfo(address, NULL, &hints, &resultList) == 0)
			{
				for (addrinfo* result = resultList; result != NULL; result = result->ai_next)
				{
					if (result->ai_family == AF_INET && result->ai_addr != NULL && result->ai_addrlen >= sizeof(struct sockaddr_in))
					{
						PUSH_CLANG_WARNINGS
						DISABLE_CLANG_WARNING("-Wcast-align")
						struct sockaddr_in* sin = (struct sockaddr_in*)result->ai_addr;
						POP_CLANG_WARNINGS
						*this = IPAddress4{sin->sin_addr.s_addr};
						freeaddrinfo(resultList);
						return;
					}
					else if (result->ai_family == AF_INET6 && result->ai_addr != NULL && result->ai_addrlen >= sizeof(struct sockaddr_in6))
					{
						PUSH_CLANG_WARNINGS
						DISABLE_CLANG_WARNING("-Wcast-align")
						struct sockaddr_in6* sin = (struct sockaddr_in6*)result->ai_addr;
						POP_CLANG_WARNINGS
						static_assert(sizeof(sin->sin6_addr) == sizeof(uint128));
						*this = IPAddress6{reinterpret_cast<const uint128&>(sin->sin6_addr)};
						freeaddrinfo(resultList);
						return;
					}
				}

				if (resultList != nullptr)
				{
					freeaddrinfo(resultList);
				}
			}
#elif PLATFORM_POSIX
			struct hostent* hostEntry = gethostbyname(address);
			if (hostEntry != nullptr)
			{
				if (hostEntry->h_addrtype == AF_INET)
				{
					PUSH_CLANG_WARNINGS
					DISABLE_CLANG_WARNING("-Wcast-align")
					const uint32 ipv4Address = *(uint32*)hostEntry->h_addr_list[0];
					*this = IPAddress4{ipv4Address};
					POP_CLANG_WARNINGS
					return;
				}
				else if (hostEntry->h_addrtype == AF_INET6)
				{
					PUSH_CLANG_WARNINGS
					DISABLE_CLANG_WARNING("-Wcast-align")
					const uint128 ipv6Address = *(uint128*)hostEntry->h_addr_list[0];
					*this = IPAddress6{ipv6Address};
					POP_CLANG_WARNINGS
					return;
				}
			}
#else
#error "Not implemented for platform"
#endif
		}
	}

	IPAddress IPAddress::GetReverseLookupAddress() const
	{
		switch (m_type)
		{
			case Type::Invalid:
				return {};
			case Type::IPv4:
				return m_ipv4.GetReverseLookupAddress();
			case Type::IPv6:
				return m_ipv6.GetReverseLookupAddress();
		}
		ExpectUnreachable();
	}

	IO::URI IPAddress::ToURI() const
	{
		switch (m_type)
		{
			case Type::Invalid:
				return {};
			case Type::IPv4:
				return m_ipv4.ToURI();
			case Type::IPv6:
				return m_ipv6.ToURI();
		}
		ExpectUnreachable();
	}

	Address::Address(const IO::ConstZeroTerminatedURIView address)
	{
		using ViewType = IO::ConstZeroTerminatedURIView::ViewType;
		const ViewType::SizeType delimiterPosition = address.GetView().FindFirstOf(MAKE_URI_LITERAL(':'));
		if (delimiterPosition != ViewType::InvalidPosition)
		{
			m_ipAddress = IPAddress(IO::URI::StringType(address.GetView().GetSubstringUpTo(delimiterPosition)));
			m_port = Port(address.GetView().GetSubstringFrom(delimiterPosition + 1).ToIntegral<uint16>());
		}
		else
		{
			m_ipAddress = IPAddress(address);
			m_port = Port::Default();
		}
	}

	IO::URI Address::ToURI() const
	{
		IO::URI::StringType uriString;
		uriString.Format("{}:{}", m_ipAddress.ToURI(), m_port.Get());
		return IO::URI(Move(uriString));
	}
}

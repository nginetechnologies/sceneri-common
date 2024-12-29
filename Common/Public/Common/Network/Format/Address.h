#pragma once

#include "../Address.h"
#include "Port.h"

#include <Common/3rdparty/fmt/Include.h>
#include <Common/IO/Format/URI.h>
#include <Common/IO/Format/ZeroTerminatedURIView.h>
#include <Common/Memory/Containers/Format/StringView.h>

namespace fmt
{
	template<>
	struct formatter<ngine::Network::IPAddress>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Network::IPAddress& ipAddress, FormatContext& ctx) const
		{
			format_to(ctx.out(), "{}", ipAddress.ToURI());
			return ctx.out();
		}
	};

	template<>
	struct formatter<ngine::Network::Address>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Network::Address& address, FormatContext& ctx) const
		{
			format_to(ctx.out(), "{}", address.ToURI());
			return ctx.out();
		}
	};
}

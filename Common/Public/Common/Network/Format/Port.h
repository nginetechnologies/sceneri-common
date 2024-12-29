#pragma once

#include "../Port.h"

#include <Common/3rdparty/fmt/Include.h>

namespace fmt
{
	template<>
	struct formatter<ngine::Network::Port>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Network::Port& port, FormatContext& ctx) const
		{
			format_to(ctx.out(), "{}", port.Get());
			return ctx.out();
		}
	};
}

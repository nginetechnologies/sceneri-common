#pragma once

#include "../Guid.h"
#include <Common/Memory/Containers/Format/String.h>

namespace fmt
{
	template<>
	struct formatter<ngine::Guid>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Guid& guid, FormatContext& ctx) const
		{
			return format_to(ctx.out(), FMT_COMPILE("{}"), guid.ToString());
		}
	};
}

#pragma once

#include "../Timestamp.h"

#include <Common/Memory/Containers/Format/StringView.h>

namespace fmt
{
	template<>
	struct formatter<ngine::Time::Timestamp>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Time::Timestamp& timestamp, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", timestamp.ToString().GetView());
		}
	};
}

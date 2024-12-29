#pragma once

#include "../Color.h"

#include <Common/Memory/Containers/Format/String.h>

namespace fmt
{
	template<typename Type>
	struct formatter<ngine::Math::TColor<Type>>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Math::TColor<Type>& color, FormatContext& ctx) const
		{
			return format_to(ctx.out(), FMT_COMPILE("{}"), color.ToString());
		}
	};
}

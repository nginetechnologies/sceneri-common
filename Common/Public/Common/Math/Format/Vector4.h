#pragma once

#include "../Vector4.h"

#include <Common/Memory/Containers/Format/String.h>

namespace fmt
{
	template<typename Type>
	struct formatter<ngine::Math::TVector4<Type>>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::Math::TVector4<Type>& vector, FormatContext& ctx) const
		{
			return format_to(ctx.out(), FMT_COMPILE("{0}, {1}, {2}, {3}"), vector.x, vector.y, vector.z, vector.w);
		}
	};
}

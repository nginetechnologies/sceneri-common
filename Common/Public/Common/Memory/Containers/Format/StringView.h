#pragma once

#include "../StringView.h"
#include "String.h"

#include <Common/3rdparty/fmt/Include.h>

namespace fmt
{
	template<typename CharType, typename SizeType>
	struct formatter<ngine::TStringView<CharType, SizeType>>
	{
		template<typename ParseContext>
		inline constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		inline auto format(const ngine::TStringView<CharType, SizeType> stringView, FormatContext& ctx) const
		{
			if (stringView.HasElements())
			{
				if constexpr (ngine::TypeTraits::IsSame<ngine::TypeTraits::WithoutConst<CharType>, char>)
				{
					return format_to(ctx.out(), FMT_COMPILE("{:.{}}"), stringView.GetData(), stringView.GetSize());
				}
				else
				{
					ngine::String tempString(stringView);
					return format_to(ctx.out(), FMT_COMPILE("{}"), tempString.GetData(), tempString.GetSize());
				}
			}
			else
			{
				return ctx.out();
			}
		}
	};

	template<typename CharType, typename InternalSizeType>
	struct formatter<ngine::TZeroTerminatedStringView<CharType, InternalSizeType>>
	{
		template<typename ParseContext>
		inline constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		inline auto format(const ngine::TZeroTerminatedStringView<CharType, InternalSizeType> stringView, FormatContext& ctx) const
		{
			if (stringView.HasElements())
			{
				if constexpr (ngine::TypeTraits::IsSame<ngine::TypeTraits::WithoutConst<CharType>, char>)
				{
					return format_to(ctx.out(), FMT_COMPILE("{:.{}}"), stringView.GetData(), stringView.GetSize());
				}
				else
				{
					ngine::String tempString(stringView);
					return format_to(ctx.out(), FMT_COMPILE("{:.{}}"), tempString.GetData(), tempString.GetSize());
				}
			}
			else
			{
				return ctx.out();
			}
		}
	};
}

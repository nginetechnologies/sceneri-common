#pragma once

#include "../URI.h"

#include <Common/3rdparty/fmt/Include.h>

namespace fmt
{
	template<>
	struct formatter<ngine::IO::URI>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::IO::URI& path, FormatContext& ctx) const
		{
			if (path.HasElements())
			{
				if constexpr (ngine::TypeTraits::IsSame<ngine::IO::URI::CharType, char>)
				{
					return format_to(ctx.out(), "{:.{}}", path.GetZeroTerminated().GetData(), path.GetSize());
				}
				else
				{
					using AllocatorType = ngine::IO::URI::StringType::AllocatorType::Rebind<char>;
					ngine::TString<char, AllocatorType, 0> convertedPath(path.GetView().GetStringView());
					return format_to(ctx.out(), "{:.{}}", convertedPath.GetData(), convertedPath.GetSize());
				}
			}
			return ctx.out();
		}
	};

	template<>
	struct formatter<ngine::IO::URIView>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::IO::URIView& stringView, FormatContext& ctx) const
		{
			if constexpr (ngine::TypeTraits::IsSame<ngine::IO::URIView::CharType, char>)
			{
				return format_to(ctx.out(), "{:.{}}", stringView.GetData(), stringView.GetSize());
			}
			else
			{
				using AllocatorType = ngine::IO::URI::StringType::AllocatorType::Rebind<char>;
				ngine::TString<char, AllocatorType, 0> convertedPath(stringView.GetStringView());
				return format_to(ctx.out(), "{:.{}}", convertedPath.GetData(), convertedPath.GetSize());
			}
		}
	};
}

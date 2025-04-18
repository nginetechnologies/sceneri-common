#pragma once

#include "../ForwardDeclarations/ZeroTerminatedURIView.h"

#include <Common/3rdparty/fmt/Include.h>

namespace fmt
{
	template<>
	struct formatter<ngine::IO::ZeroTerminatedURIView>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::IO::ZeroTerminatedURIView pathView, FormatContext& ctx) const
		{
			if (pathView.HasElements())
			{
				if constexpr (ngine::TypeTraits::IsSame<ngine::IO::URIView::CharType, char>)
				{
					return format_to(ctx.out(), "{:.{}}", pathView.GetData(), pathView.GetSize());
				}
				else
				{
					using AllocatorType = ngine::IO::URI::StringType::AllocatorType::Rebind<char>;
					ngine::TString<char, AllocatorType, 0> convertedPath(ngine::IO::URIView::ConstStringViewType(pathView, pathView.GetSize()));
					return format_to(ctx.out(), "{:.{}}", convertedPath.GetData(), convertedPath.GetSize());
				}
			}
			else
			{
				return ctx.out();
			}
		}
	};

	template<>
	struct formatter<ngine::IO::ConstZeroTerminatedURIView>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::IO::ConstZeroTerminatedURIView pathView, FormatContext& ctx) const
		{
			if constexpr (ngine::TypeTraits::IsSame<ngine::IO::URIView::CharType, char>)
			{
				return format_to(ctx.out(), "{:.{}}", pathView.GetData(), pathView.GetSize());
			}
			else
			{
				using AllocatorType = ngine::IO::URI::StringType::AllocatorType::Rebind<char>;
				ngine::TString<char, AllocatorType, 0> convertedPath(ngine::IO::URIView::ConstStringViewType(pathView, pathView.GetSize()));
				return format_to(ctx.out(), "{:.{}}", convertedPath.GetData(), convertedPath.GetSize());
			}
		}
	};
}

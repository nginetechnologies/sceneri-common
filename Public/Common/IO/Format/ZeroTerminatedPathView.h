#pragma once

#include "../ForwardDeclarations/ZeroTerminatedPathView.h"

#include <Common/3rdparty/fmt/Include.h>

namespace fmt
{
	template<>
	struct formatter<ngine::IO::ZeroTerminatedPathView>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::IO::ZeroTerminatedPathView pathView, FormatContext& ctx) const
		{
			if (pathView.HasElements())
			{
				if constexpr (ngine::TypeTraits::IsSame<ngine::IO::PathView::CharType, char>)
				{
					return format_to(ctx.out(), "{:.{}}", pathView.GetData(), pathView.GetSize());
				}
				else
				{
					using AllocatorType = ngine::IO::Path::StringType::AllocatorType::Rebind<char>;
					ngine::TString<char, AllocatorType, 0> convertedPath(ngine::IO::PathView::ConstStringViewType(pathView, pathView.GetSize()));
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
	struct formatter<ngine::IO::ConstZeroTerminatedPathView>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::IO::ConstZeroTerminatedPathView pathView, FormatContext& ctx) const
		{
			if constexpr (ngine::TypeTraits::IsSame<ngine::IO::PathView::CharType, char>)
			{
				return format_to(ctx.out(), "{:.{}}", pathView.GetData(), pathView.GetSize());
			}
			else
			{
				using AllocatorType = ngine::IO::Path::StringType::AllocatorType::Rebind<char>;
				ngine::TString<char, AllocatorType, 0> convertedPath(ngine::IO::PathView::ConstStringViewType(pathView, pathView.GetSize()));
				return format_to(ctx.out(), "{:.{}}", convertedPath.GetData(), convertedPath.GetSize());
			}
		}
	};
}

#pragma once

#include "../String.h"

#include <Common/3rdparty/fmt/Include.h>

namespace ngine::FormatInternal
{
	template<typename StringType, typename... Args>
	inline void FormatString(StringType& string, const TStringView<const char> format, Args&&... args)
	{
		using fmt_string_view = fmt::basic_string_view<TypeTraits::WithoutConst<typename decltype(format)::CharType>>;
		const typename StringType::SizeType formattedSize =
			static_cast<typename StringType::SizeType>(fmt::formatted_size(fmt_string_view(format.GetData(), format.GetSize()), args...));

		string.Resize(formattedSize);
		fmt::vformat_to_n<typename StringType::CharType*, char, Args...>(
			string.GetData(),
			string.GetCapacity(),
			fmt_string_view(format.GetData(), format.GetSize()),
			fmt::make_format_args(args...)
		);
	}
}

namespace fmt
{
	template<typename CharType, typename AllocatorType, ngine::uint8 Flags>
	struct formatter<ngine::TString<CharType, AllocatorType, Flags>>
	{
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}

		template<typename FormatContext>
		auto format(const ngine::TString<CharType, AllocatorType, Flags>& string, FormatContext& ctx) const
		{
			if (string.HasElements())
			{
				if constexpr (ngine::TypeTraits::IsSame<CharType, char>)
				{
					return format_to(ctx.out(), "{:.{}}", string.GetData(), string.GetSize());
				}
				else
				{
					using NewAllocatorType = typename AllocatorType::template Rebind<char>;
					ngine::TString<char, NewAllocatorType, 0> convertedPath(string.GetView());
					return format_to(ctx.out(), "{:.{}}", convertedPath.GetData(), convertedPath.GetSize());
				}
			}
			else
			{
				return ctx.out();
			}
		}
	};
}

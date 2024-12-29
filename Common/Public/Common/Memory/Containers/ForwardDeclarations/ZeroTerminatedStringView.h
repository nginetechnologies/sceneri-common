#pragma once

#include <Common/Memory/NativeCharType.h>
#include <Common/Memory/UnicodeCharType.h>

namespace ngine
{
	template<typename CharType, typename SizeType = uint32>
	struct TZeroTerminatedStringView;

	using ZeroTerminatedStringView = TZeroTerminatedStringView<const char>;
	using ConstZeroTerminatedStringView = TZeroTerminatedStringView<const char>;
	using ZeroTerminatedWideStringView = TZeroTerminatedStringView<const wchar_t>;
	using ConstZeroTerminatedWideStringView = TZeroTerminatedStringView<const wchar_t>;
	using ZeroTerminatedUnicodeStringView = TZeroTerminatedStringView<const UnicodeCharType>;
	using ConstZeroTerminatedUnicodeStringView = TZeroTerminatedStringView<const UnicodeCharType>;

	using ConstNativeZeroTerminatedStringView = TZeroTerminatedStringView<const NativeCharType>;
	using NativeZeroTerminatedStringView = TZeroTerminatedStringView<NativeCharType>;
}

namespace fmt
{
	inline namespace v11
	{
		template<typename T, typename Char, typename Enable>
		struct formatter;

		template<typename CharType, typename InternalSizeType>
		struct formatter<ngine::TZeroTerminatedStringView<CharType, InternalSizeType>, char, void>;
	}
}

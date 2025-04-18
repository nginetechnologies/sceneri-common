#pragma once

#include <Common/Memory/NativeCharType.h>
#include <Common/Memory/UnicodeCharType.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	template<typename InternalCharType, typename InternalSizeType = uint32>
	struct TStringView;

	using StringView = TStringView<char>;
	using ConstStringView = TStringView<const char>;

	using WideStringView = TStringView<wchar_t>;
	using ConstWideStringView = TStringView<const wchar_t>;

	using UTF16StringView = TStringView<char16_t>;
	using ConstUTF16StringView = TStringView<const char16_t>;

	using UTF32StringView = TStringView<char32_t>;
	using ConstUTF32StringView = TStringView<const char32_t>;

	using UnicodeStringView = TStringView<UnicodeCharType>;
	using ConstUnicodeStringView = TStringView<const UnicodeCharType>;

	using NativeStringView = TStringView<NativeCharType>;
	using ConstNativeStringView = TStringView<const NativeCharType>;
}

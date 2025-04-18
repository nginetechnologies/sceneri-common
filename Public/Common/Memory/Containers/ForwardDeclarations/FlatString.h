#pragma once

#include "StringBase.h"
#include <Common/Memory/Allocators/FixedAllocator.h>
#include <Common/Memory/UnicodeCharType.h>
#include <Common/Memory/NativeCharType.h>

#include <Common/Memory/Containers/VectorFlags.h>

namespace ngine
{
	template<typename CharType, size Capacity, uint8 Flags>
	using TFlatString = TString<CharType, Memory::FixedAllocator<CharType, Capacity>, Flags>;

	template<size Capacity>
	using FlatString = TFlatString<char, Capacity, Memory::VectorFlags::AllowResize>;
	template<size Capacity>
	using FixedSizeFlatString = TFlatString<char, Capacity, Memory::VectorFlags::None>;

	template<size Capacity>
	using FlatWideString = TFlatString<wchar_t, Capacity, Memory::VectorFlags::AllowResize>;
	template<size Capacity>
	using FixedSizeFlatWideString = TFlatString<wchar_t, Capacity, Memory::VectorFlags::None>;

	template<size Capacity>
	using FlatUnicodeString = TFlatString<UnicodeCharType, Capacity, Memory::VectorFlags::AllowResize>;
	template<size Capacity>
	using FixedSizeFlatUnicodeString = TFlatString<UnicodeCharType, Capacity, Memory::VectorFlags::None>;

	template<size Capacity>
	using FlatNativeString = TFlatString<NativeCharType, Capacity, Memory::VectorFlags::AllowResize>;
	template<size Capacity>
	using FixedSizeFlatNativeString = TFlatString<NativeCharType, Capacity, Memory::VectorFlags::None>;
}

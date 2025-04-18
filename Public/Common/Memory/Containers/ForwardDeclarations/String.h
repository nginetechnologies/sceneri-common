#pragma once

#include "../ForwardDeclarations/StringBase.h"

#include <Common/Memory/Allocators/ForwardDeclarations/DynamicInlineStorageAllocator.h>
#include <Common/Memory/Containers/ForwardDeclarations/StringView.h>
#include <Common/Memory/Containers/VectorFlags.h>
#include <Common/Memory/NativeCharType.h>
#include <Common/Memory/UnicodeCharType.h>

namespace ngine
{
	namespace Memory::Internal
	{
		inline static constexpr size SmallStringOptimizationBufferSize = 16;
		inline static constexpr size SmallPathOptimizationBufferSize = 250;
	}

	using String = TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	using FixedCapacityString = TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	using FixedSizeString = TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;

	using WideString = TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	using FixedCapacityWideString = TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	using FixedSizeWideString = TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;

	using UTF8String = TString<
		UTF8CharType,
		Memory::DynamicInlineStorageAllocator<UTF8CharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	using FixedCapacityUTF8String = TString<
		UTF8CharType,
		Memory::DynamicInlineStorageAllocator<UTF8CharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	using FixedSizeUTF8String = TString<
		UTF8CharType,
		Memory::DynamicInlineStorageAllocator<UTF8CharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;

	using UTF16String = TString<
		char16_t,
		Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	using FixedCapacityUTF16String = TString<
		char16_t,
		Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	using FixedSizeUTF16String = TString<
		char16_t,
		Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;

	using UTF32String = TString<
		char32_t,
		Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	using FixedCapacityUTF32String = TString<
		char32_t,
		Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	using FixedSizeUTF32String = TString<
		char32_t,
		Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;

	using UnicodeString = UTF16String;
	using FixedCapacityUnicodeString = FixedCapacityUTF16String;
	using FixedSizeUnicodeString = FixedSizeUTF16String;

	using NativeString = TString<
		NativeCharType,
		Memory::DynamicInlineStorageAllocator<NativeCharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
}

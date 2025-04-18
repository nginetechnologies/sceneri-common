#pragma once

#include <Common/Memory/Allocators/DynamicInlineStorageAllocator.h>
#include "StringBase.h"

#include "ForwardDeclarations/String.h"

#include <Common/IO/PathConstants.h>

namespace ngine
{
	extern template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	extern template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	extern template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	extern template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	extern template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	extern template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	extern template struct TString<
		UnicodeCharType,
		Memory::DynamicAllocator<UnicodeCharType, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	extern template struct TString<UnicodeCharType, Memory::DynamicAllocator<UnicodeCharType, uint32>, Memory::VectorFlags::AllowResize>;
	extern template struct TString<UnicodeCharType, Memory::DynamicAllocator<UnicodeCharType, uint32>, Memory::VectorFlags::None>;

	extern template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallPathOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	extern template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallPathOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
}

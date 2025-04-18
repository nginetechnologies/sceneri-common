#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/NoDebug.h>

#if PLATFORM_WINDOWS
extern "C"
{
	[[nodiscard]] int __cdecl memcmp(const void* ptr1, const void* ptr2, size_t num);
}
#endif

namespace ngine::Memory
{
	[[nodiscard]] FORCE_INLINE NO_DEBUG int Compare(const void* pLeft, const void* pRight, const size size) noexcept
	{
#if COMPILER_CLANG || COMPILER_GCC
		return __builtin_memcmp(pLeft, pRight, size);
#else
		return memcmp(pLeft, pRight, size);
#endif
	}
}

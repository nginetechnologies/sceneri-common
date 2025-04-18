#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/Assume.h>

#if PLATFORM_WINDOWS
extern "C"
{
	[[nodiscard]] void* __cdecl memset(void* ptr, const int value, size_t num);
}
#endif

namespace ngine::Memory
{
	template<size Count>
	FORCE_INLINE void Set(void* pDestination, const int value) noexcept
	{
		ASSUME(pDestination != nullptr || Count == 0);
#if COMPILER_CLANG || COMPILER_GCC
		__builtin_memset(pDestination, value, Count);
#else
		memset(pDestination, value, Count);
#endif
	}

	FORCE_INLINE void Set(void* pDestination, const int value, const size count) noexcept
	{
		ASSUME(pDestination != nullptr || count == 0);
#if COMPILER_CLANG || COMPILER_GCC
		__builtin_memset(pDestination, value, count);
#else
		memset(pDestination, value, count);
#endif
	}
}

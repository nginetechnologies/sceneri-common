#pragma once

#include <Common/Platform/NoDebug.h>

namespace ngine::Memory
{
	enum class PrefetchType : uint8
	{
		Read = 0,
		Write
	};

	enum class PrefetchLocality : uint8
	{
#if COMPILER_CLANG || COMPILER_GCC
		RemoveAfterAccess = 0,
		LowPriority,
		MediumPriority,
		KeepInAllPossibleCacheLevels
#else // if USE_SSE
		RemoveAfterAccess = 0,           //_MM_HINT_NTA,
		LowPriority = 3,                 //_MM_HINT_T2,
		MediumPriority = 2,              //_MM_HINT_T1,
		KeepInAllPossibleCacheLevels = 1 //_MM_HINT_T0
#endif
	};

	template<PrefetchType ReadType, PrefetchLocality LocalityType>
	FORCE_INLINE NO_DEBUG void PrefetchLine([[maybe_unused]] const void* pSource) noexcept
	{
#if COMPILER_CLANG || COMPILER_GCC
		static_assert((uint8)ReadType <= 1);
		static_assert((uint8)LocalityType <= 3);

		__builtin_prefetch(pSource, (uint8)ReadType, (uint8)LocalityType);
#elif USE_SSE
		_mm_prefetch(static_cast<const char*>(pSource), (uint8)LocalityType);
#endif
	}
}

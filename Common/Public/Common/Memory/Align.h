#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/NoDebug.h>

namespace ngine::Memory
{
	template<typename Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr Type Align(const Type value, const size alignment)
	{
#if COMPILER_CLANG
		return __builtin_align_up(value, alignment);
#else
		return static_cast<Type>(value + (alignment - 1)) & (0 - alignment);
#endif
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG Type* Align(Type* const pAddress, const size alignment)
	{
#if COMPILER_CLANG
		return __builtin_align_up(pAddress, alignment);
#else
		return reinterpret_cast<Type*>((reinterpret_cast<uintptr>(pAddress) + (alignment - 1)) & (0 - alignment));
#endif
	}
}

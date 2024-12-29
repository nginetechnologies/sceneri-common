#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS NO_DEBUG constexpr bool IsAligned(const Type value, const size alignment)
	{
#if COMPILER_CLANG
		return __builtin_is_aligned(value, alignment);
#else
		return (value & (alignment - 1)) == 0;
#endif
	}
	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS NO_DEBUG bool IsAligned(Type* const pAddress, const size alignment)
	{
#if COMPILER_CLANG
		return __builtin_is_aligned(pAddress, alignment);
#else
		return (reinterpret_cast<uintptr>(pAddress) & (alignment - 1)) == 0;
#endif
	}

	template<typename Type>
	[[nodiscard]] constexpr FORCE_INLINE PURE_STATICS NO_DEBUG bool IsAligned(const uintptr pAddress)
	{
		return IsAligned(pAddress, alignof(Type));
	}
	template<typename Type, typename PointerType>
	[[nodiscard]] constexpr FORCE_INLINE PURE_STATICS NO_DEBUG bool IsAligned(const PointerType* pAddress)
	{
		return IsAligned<Type>(reinterpret_cast<uintptr>(pAddress));
	}
}

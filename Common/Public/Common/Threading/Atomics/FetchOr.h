#pragma once

#if COMPILER_MSVC
extern "C"
{
	char _InterlockedOr8(char volatile *, char);
	short _InterlockedOr16(short volatile *, short);
	long _InterlockedOr(long volatile *, long);
	__int64 _InterlockedOr64(__int64 volatile *, __int64);
};

#pragma intrinsic(_InterlockedOr8)
#pragma intrinsic(_InterlockedOr16)
#pragma intrinsic(_InterlockedOr)
#if defined(PLATFORM_64BIT)
#pragma intrinsic(_InterlockedOr64)
#endif // PLATFORM_64BIT

#elif !COMPILER_CLANG && !COMPILER_GCC // COMPILER_MSVC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE Type FetchOr(Type& storedValue, const Type mask)
	{
#if COMPILER_MSVC
		if constexpr (sizeof(Type) == 1)
		{
			return static_cast<Type>(_InterlockedOr8(reinterpret_cast<volatile char*>(&storedValue), static_cast<char>(mask)));
		}
		else if constexpr (sizeof(Type) == 2)
		{
			return static_cast<Type>(_InterlockedOr16(reinterpret_cast<volatile short*>(&storedValue), static_cast<short>(mask)));
		}
		else if constexpr (sizeof(Type) == 4)
		{
			return static_cast<Type>(_InterlockedOr(reinterpret_cast<volatile long*>(&storedValue), static_cast<long>(mask)));
		}
		else if constexpr (sizeof(Type) == 8)
		{
			return static_cast<Type>(_InterlockedOr64(reinterpret_cast<volatile __int64*>(&storedValue), static_cast<__int64>(mask)));
		}
		else
		{
			static_unreachable("Unsupported type size!");
		}
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_fetch_or(&storedValue, mask, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<std::atomic<Type>&>(storedValue).fetch_or(mask);
#endif
	}
}

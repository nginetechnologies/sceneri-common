#pragma once

#if COMPILER_MSVC
extern "C"
{
	short _InterlockedIncrement16(short volatile *);
	long _InterlockedIncrement(long volatile *);
	__int64 _InterlockedIncrement64(__int64 volatile *);
};

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedIncrement16)
#if defined(PLATFORM_64BIT)
#pragma intrinsic(_InterlockedIncrement64)
#endif // PLATFORM_64BIT

#include "CompareExchangeStrong.h"

#elif !COMPILER_CLANG && !COMPILER_GCC // COMPILER_MSVC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE Type FetchIncrement(Type& storedValue)
	{
#if COMPILER_MSVC
		if constexpr (sizeof(Type) == 1)
		{
			// There's no _InterlockedIncrement8().
			char previousValue, newValue;
			do
			{
				previousValue = static_cast<char>(storedValue);
				newValue = previousValue + static_cast<char>(1);
			} while (_InterlockedCompareExchange8(reinterpret_cast<volatile char*>(&storedValue), newValue, previousValue) != previousValue);
			return static_cast<Type>(newValue) - 1;
		}
		else if constexpr (sizeof(Type) == 2)
		{
			return static_cast<Type>(_InterlockedIncrement16(reinterpret_cast<volatile short*>(&storedValue))) - 1;
		}
		else if constexpr (sizeof(Type) == 4)
		{
			return static_cast<Type>(_InterlockedIncrement(reinterpret_cast<volatile long*>(&storedValue))) - 1;
		}
		else if constexpr (sizeof(Type) == 8)
		{
			return static_cast<Type>(_InterlockedIncrement64(reinterpret_cast<volatile __int64*>(&storedValue))) - 1;
		}
		else
		{
			static_unreachable("Unsupported type size!");
		}
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_fetch_add(&storedValue, 1, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<std::atomic<Type>&>(storedValue).fetch_add(1);
#endif
	}
}

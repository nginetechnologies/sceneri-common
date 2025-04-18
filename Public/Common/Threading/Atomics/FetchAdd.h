#pragma once

#include <Common/Platform/ForceInline.h>

#if COMPILER_MSVC
extern "C"
{
	short _InterlockedExchangeAdd16(short volatile *, short);
	long _InterlockedExchangeAdd(long volatile *, long);
	__int64 _InterlockedExchangeAdd64(__int64 volatile *, __int64);
};

#pragma intrinsic(_InterlockedExchangeAdd16)
#pragma intrinsic(_InterlockedExchangeAdd)
#if PLATFORM_64BIT
#pragma intrinsic(_InterlockedExchangeAdd64)
#endif // PLATFORM_64BIT

#include "CompareExchangeStrong.h"

#elif !COMPILER_CLANG && !COMPILER_GCC // COMPILER_MSVC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE Type FetchAdd(Type& storedValue, const Type value)
	{
#if COMPILER_MSVC
		if constexpr (sizeof(Type) == 1)
		{
			char previousValue, newValue;
			do
			{
				previousValue = static_cast<char>(storedValue);
				newValue = previousValue + static_cast<char>(value);
			} while (_InterlockedCompareExchange8(reinterpret_cast<volatile char*>(&storedValue), newValue, previousValue) != previousValue);
			return static_cast<Type>(previousValue);
		}
		else if constexpr (sizeof(Type) == 2)
		{
			return static_cast<Type>(_InterlockedExchangeAdd16(reinterpret_cast<volatile short*>(&storedValue), static_cast<short>(value)));
		}
		else if constexpr (sizeof(Type) == 4)
		{
			return static_cast<Type>(_InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&storedValue), static_cast<long>(value)));
		}
		else if constexpr (sizeof(Type) == 8)
		{
			return static_cast<Type>(_InterlockedExchangeAdd64(reinterpret_cast<volatile __int64*>(&storedValue), static_cast<__int64>(value)));
		}
		else
		{
			static_unreachable("Unsupported type size!");
		}
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_fetch_add(&storedValue, value, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<std::atomic<Type>&>(storedValue).fetch_add(value);
#endif
	}
}

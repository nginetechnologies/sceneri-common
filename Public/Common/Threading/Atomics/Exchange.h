#pragma once

#if COMPILER_MSVC
#include <Common/TypeTraits/IsPointer.h>
#include <Common/Platform/StaticUnreachable.h>

extern "C"
{
	char _InterlockedExchange8(char volatile *, char);
	short _InterlockedExchange16(short volatile *, short);
	long _InterlockedExchange(long volatile *, long);
	__int64 _InterlockedExchange64(__int64 volatile *, __int64);
};
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchange8)
#pragma intrinsic(_InterlockedExchange16)
#if PLATFORM_64BIT
#pragma intrinsic(_InterlockedExchange64)
#endif // PLATFORM_64BIT

#elif !COMPILER_CLANG && !COMPILER_GCC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE Type Exchange(Type& value, const Type newValue)
	{
#if COMPILER_MSVC
		if constexpr (sizeof(Type) == 1)
		{
			return static_cast<Type>(_InterlockedExchange8(reinterpret_cast<volatile char*>(&value), static_cast<const char>(newValue)));
		}
		else if constexpr (sizeof(Type) == 2)
		{
			return static_cast<Type>(_InterlockedExchange16(reinterpret_cast<volatile short*>(&value), static_cast<const short>(newValue)));
		}
		else if constexpr (sizeof(Type) == 4)
		{
			return static_cast<Type>(_InterlockedExchange(reinterpret_cast<volatile long*>(&value), static_cast<const long>(newValue)));
		}
		else if constexpr (sizeof(Type) == 8)
		{
			return static_cast<Type>(_InterlockedExchange64(reinterpret_cast<volatile __int64*>(&value), static_cast<const __int64>(newValue)));
		}
		else
		{
			static_unreachable("Unsupported type size!");
		}
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_exchange_n(&value, newValue, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<std::atomic<Type>&>(value).exchange(newValue);
#endif
	}

	template<typename Type>
	FORCE_INLINE Type* Exchange(Type*& value, Type* const newValue)
	{
		return reinterpret_cast<Type*>(Exchange<uintptr>(reinterpret_cast<uintptr&>(value), reinterpret_cast<uintptr>(newValue)));
	}
}

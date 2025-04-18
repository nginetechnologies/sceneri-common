#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Math/CoreNumericTypes.h>

#if COMPILER_MSVC
extern "C"
{
	char _InterlockedCompareExchange8(char volatile *, char, char);
	short _InterlockedCompareExchange16(short volatile *, short, short);
	long _InterlockedCompareExchange(long volatile *, long, long);
	__int64 _InterlockedCompareExchange64(__int64 volatile *, __int64, __int64);
	unsigned char _InterlockedCompareExchange128(__int64 volatile *, __int64, __int64, __int64*);
};

#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange8)
#pragma intrinsic(_InterlockedCompareExchange16)
#if PLATFORM_64BIT
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedCompareExchange128)
#endif // PLATFORM_64BIT

#elif !COMPILER_CLANG && !COMPILER_GCC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE bool CompareExchangeStrong(Type& value, Type& expected, Type desired)
	{
#if COMPILER_MSVC
		if constexpr (sizeof(Type) == 1)
		{
			const char expectedData = static_cast<char>(expected);
			const char previousData =
				_InterlockedCompareExchange8(reinterpret_cast<volatile char*>(&value), static_cast<const char>(desired), expectedData);
			if (previousData == expectedData)
			{
				return true;
			}
			reinterpret_cast<char&>(expected) = previousData;
			return false;
		}
		else if constexpr (sizeof(Type) == 2)
		{
			const short expectedData = static_cast<short>(expected);
			const short previousData =
				_InterlockedCompareExchange16(reinterpret_cast<volatile short*>(&value), static_cast<const short>(desired), expectedData);
			if (previousData == expectedData)
			{
				return true;
			}
			reinterpret_cast<short&>(expected) = previousData;
			return false;
		}
		else if constexpr (sizeof(Type) == 4)
		{
			const long expectedData = static_cast<long>(expected);
			const long previousData =
				_InterlockedCompareExchange(reinterpret_cast<volatile long*>(&value), static_cast<const long>(desired), expectedData);
			if (previousData == expectedData)
			{
				return true;
			}
			reinterpret_cast<long&>(expected) = previousData;
			return false;
		}
		else if constexpr (sizeof(Type) == 8)
		{
			const __int64 expectedData = static_cast<__int64>(expected);
			const __int64 previousData =
				_InterlockedCompareExchange64(reinterpret_cast<volatile __int64*>(&value), static_cast<const __int64>(desired), expectedData);
			if (previousData == expectedData)
			{
				return true;
			}
			reinterpret_cast<__int64&>(expected) = previousData;
			return false;
		}
		else if constexpr (sizeof(Type) == 16)
		{
			uint128 desiredData = reinterpret_cast<uint128&>(desired);
			uint128& expectedData = reinterpret_cast<uint128&>(expected);
			return (bool)_InterlockedCompareExchange128(
				reinterpret_cast<volatile int64*>(&value),
				desiredData.m_highPart,
				desiredData.m_lowPart,
				&reinterpret_cast<int64&>(expectedData)
			);
		}
		else
		{
			static_unreachable("Unsupported type size!");
		}
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_compare_exchange(&value, &expected, &desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<std::atomic<Type>&>(value).compare_exchange_strong(expected, desired);
#endif
	}

	template<typename Type>
	FORCE_INLINE bool CompareExchangeStrong(Type*& value, Type*& expected, Type* const desired)
	{
		return CompareExchangeStrong<uintptr>(
			reinterpret_cast<uintptr&>(value),
			reinterpret_cast<uintptr&>(expected),
			reinterpret_cast<uintptr>(desired)
		);
	}
}

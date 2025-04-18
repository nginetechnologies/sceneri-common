#pragma once

#include <Common/Platform/NoDebug.h>
#include <Common/TypeTraits/AddConst.h>

#if COMPILER_MSVC
extern "C"
{
	char __iso_volatile_load8(const char volatile *);
	short __iso_volatile_load16(const short volatile *);
	int __iso_volatile_load32(const int volatile *);
	__int64 __iso_volatile_load64(const __int64 volatile *);
	void _ReadWriteBarrier();
};

#pragma intrinsic(__iso_volatile_load8)
#pragma intrinsic(__iso_volatile_load16)
#pragma intrinsic(__iso_volatile_load32)
#if PLATFORM_64BIT
#pragma intrinsic(__iso_volatile_load64)
#endif // PLATFORM_64BIT

#if USE_AVX
#include <emmintrin.h>
#endif

#elif !COMPILER_CLANG && !COMPILER_GCC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG Type Load(const Type& value)
	{
#if COMPILER_MSVC
		if constexpr (sizeof(Type) == 1)
		{
			const Type loadedValue = static_cast<Type>(__iso_volatile_load8(reinterpret_cast<const volatile char*>(&value)));
			_ReadWriteBarrier();
			return loadedValue;
		}
		else if constexpr (sizeof(Type) == 2)
		{
			const Type loadedValue = static_cast<Type>(__iso_volatile_load16(reinterpret_cast<const volatile short*>(&value)));
			_ReadWriteBarrier();
			return loadedValue;
		}
		else if constexpr (sizeof(Type) == 4)
		{
			const Type loadedValue = static_cast<Type>(__iso_volatile_load32(reinterpret_cast<const volatile int*>(&value)));
			_ReadWriteBarrier();
			return loadedValue;
		}
		else if constexpr (sizeof(Type) == 8)
		{
			const Type loadedValue = static_cast<Type>(__iso_volatile_load64(reinterpret_cast<const volatile __int64*>(&value)));
			_ReadWriteBarrier();
			return loadedValue;
		}
#if USE_AVX
		else if constexpr (sizeof(Type) == 16)
		{
			const __m128i result = _mm_load_si128(reinterpret_cast<const __m128i*>(&value));
			const Type loadedValue = reinterpret_cast<const Type&>(result);
			_ReadWriteBarrier();
			return loadedValue;
		}
#endif
		else
		{
			static_unreachable("Unsupported type size!");
		}
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_load_n(&value, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<const std::atomic<Type>&>(value).load();
#endif
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG Type* Load(Type* const & value)
	{
		return reinterpret_cast<Type*>(Load<uintptr>(reinterpret_cast<const uintptr&>(value)));
	}
}

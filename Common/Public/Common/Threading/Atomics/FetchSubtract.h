#pragma once

#if COMPILER_MSVC
#include "FetchAdd.h"
#include <Common/TypeTraits/MakeUnsigned.h>

#elif !COMPILER_CLANG && !COMPILER_GCC // COMPILER_MSVC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE Type FetchSubtract(Type& storedValue, const Type value)
	{
#if COMPILER_MSVC
		return FetchAdd(storedValue, static_cast<Type>(0U - static_cast<TypeTraits::Unsigned<Type>>(value)));
#elif COMPILER_CLANG || COMPILER_GCC
		return __atomic_fetch_sub(&storedValue, value, __ATOMIC_SEQ_CST);
#else
		return reinterpret_cast<std::atomic<Type>&>(storedValue).fetch_sub(value);
#endif
	}
}

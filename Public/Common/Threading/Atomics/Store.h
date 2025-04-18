#pragma once

#include <Common/Platform/NoDebug.h>

#if COMPILER_MSVC
#include "Exchange.h"
#elif !COMPILER_CLANG && !COMPILER_GCC
#include <atomic>
#endif

namespace ngine::Threading::Atomics
{
	template<typename Type>
	FORCE_INLINE NO_DEBUG void Store(Type& target, const Type& value)
	{
#if COMPILER_MSVC
		Exchange(target, value);
#elif COMPILER_CLANG || COMPILER_GCC
		__atomic_store_n(&target, value, __ATOMIC_SEQ_CST);
#else
		reinterpret_cast<std::atomic<Type>&>(target).store(value);
#endif
	}
}

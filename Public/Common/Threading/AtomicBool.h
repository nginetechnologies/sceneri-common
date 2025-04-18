#pragma once

#include <Common/Threading/AtomicBase.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Threading
{
	template<>
	struct Atomic<bool> : public Internal::LockfreeAtomic<bool>
	{
		using LockfreeAtomic::LockfreeAtomic;
		using LockfreeAtomic::operator bool;
		using LockfreeAtomic::operator=;
		using LockfreeAtomic::CompareExchangeStrong;
		using LockfreeAtomic::CompareExchangeWeak;
		using LockfreeAtomic::Exchange;

		FORCE_INLINE void operator|=(const bool value)
		{
			bool expected = false;
			CompareExchangeStrong(expected, value);
		}
	};

	extern template struct Atomic<bool>;
	extern template struct Internal::LockfreeAtomic<bool>;
}

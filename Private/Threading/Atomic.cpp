#include "Threading/AtomicBool.h"
#include "Threading/AtomicInteger.h"
#include "Threading/Mutexes/Mutex.h"
#include "Threading/Mutexes/SharedMutex.h"
#include "Threading/Mutexes/ConditionVariable.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

namespace ngine::Threading
{
	static_assert(sizeof(Atomic<bool>) == sizeof(std::atomic<bool>));
	static_assert(alignof(Atomic<bool>) == alignof(std::atomic<bool>));

	template struct Internal::LockfreeAtomic<bool>;
	template struct Internal::LockfreeAtomic<uint8>;
	template struct Internal::LockfreeAtomicWithArithmetic<uint8>;
	template struct Internal::LockfreeAtomic<uint16>;
	template struct Internal::LockfreeAtomicWithArithmetic<uint16>;
	template struct Internal::LockfreeAtomic<uint32>;
	template struct Internal::LockfreeAtomicWithArithmetic<uint32>;
	template struct Internal::LockfreeAtomic<uint64>;
	template struct Internal::LockfreeAtomicWithArithmetic<uint64>;
	template struct Internal::LockfreeAtomic<int8>;
	template struct Internal::LockfreeAtomicWithArithmetic<int8>;
	template struct Internal::LockfreeAtomic<int16>;
	template struct Internal::LockfreeAtomicWithArithmetic<int16>;
	template struct Internal::LockfreeAtomic<int32>;
	template struct Internal::LockfreeAtomicWithArithmetic<int32>;
	template struct Internal::LockfreeAtomic<int64>;
	template struct Internal::LockfreeAtomicWithArithmetic<int64>;

#if IS_SIZE_UNIQUE_TYPE
	template struct Internal::LockfreeAtomic<size>;
	template struct Internal::LockfreeAtomicWithArithmetic<size>;
#endif

	template struct Internal::LockfreeAtomic<void*>;

	template struct UniqueLock<Mutex>;
	template struct UniqueLock<SharedMutex>;
	template struct SharedLock<SharedMutex>;
}

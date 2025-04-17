#pragma once

#include <Common/Assert/Assert.h>
#include <Common/Platform/CompilerWarnings.h>
#include <Common/Platform/Likely.h>

#if PLATFORM_WINDOWS
#define USE_WINDOWS_RW_MUTEX 1
using _Smtx_t = void*;

extern "C"
{
#if (defined(_MSC_VER) && (_MSC_VER >= 1939))
	void __cdecl _Smtx_lock_exclusive(_Smtx_t*) noexcept;
	void __cdecl _Smtx_lock_shared(_Smtx_t*) noexcept;
	int __cdecl _Smtx_try_lock_exclusive(_Smtx_t*) noexcept;
	int __cdecl _Smtx_try_lock_shared(_Smtx_t*) noexcept;
	void __cdecl _Smtx_unlock_exclusive(_Smtx_t*) noexcept;
	void __cdecl _Smtx_unlock_shared(_Smtx_t*) noexcept;
#else
	void __cdecl _Smtx_lock_exclusive(_Smtx_t*);
	void __cdecl _Smtx_lock_shared(_Smtx_t*);
	int __cdecl _Smtx_try_lock_exclusive(_Smtx_t*);
	int __cdecl _Smtx_try_lock_shared(_Smtx_t*);
	void __cdecl _Smtx_unlock_exclusive(_Smtx_t*);
	void __cdecl _Smtx_unlock_shared(_Smtx_t*);
#endif
}

#elif PLATFORM_EMSCRIPTEN
#define USE_SHARED_SPINLOCK 1
#include <Common/Threading/AtomicInteger.h>
#include <Common/Threading/Mutexes/Mutex.h>
#elif PLATFORM_APPLE
#define USE_PTHREAD_SHARED_MUTEX 1

#if PLATFORM_APPLE
#include <sys/_pthread/_pthread_rwlock_t.h>
#include <pthread_impl.h>

#define PTHREAD_RWLOCK_INITIALIZER \
	{ \
		_PTHREAD_RWLOCK_SIG_init, \
		{ \
			0 \
		} \
	}
#else
#include <pthread.h>
#endif

#include <errno.h>

extern "C"
{
#if PLATFORM_APPLE
	int pthread_rwlock_destroy(pthread_rwlock_t*) __DARWIN_ALIAS(pthread_rwlock_destroy);
	int pthread_rwlock_wrlock(pthread_rwlock_t*) __DARWIN_ALIAS(pthread_rwlock_wrlock);
	int pthread_rwlock_trywrlock(pthread_rwlock_t*) __DARWIN_ALIAS(pthread_rwlock_trywrlock);
	int pthread_rwlock_unlock(pthread_rwlock_t*) __DARWIN_ALIAS(pthread_rwlock_unlock);
	int pthread_rwlock_rdlock(pthread_rwlock_t*) __DARWIN_ALIAS(pthread_rwlock_rdlock);
	int pthread_rwlock_tryrdlock(pthread_rwlock_t*) __DARWIN_ALIAS(pthread_rwlock_tryrdlock);
#else
	int pthread_rwlock_destroy(pthread_rwlock_t*);
	int pthread_rwlock_wrlock(pthread_rwlock_t*);
	int pthread_rwlock_trywrlock(pthread_rwlock_t*);
	int pthread_rwlock_unlock(pthread_rwlock_t*);
	int pthread_rwlock_rdlock(pthread_rwlock_t*);
	int pthread_rwlock_tryrdlock(pthread_rwlock_t*);
#endif
}

#elif PLATFORM_POSIX
#define USE_PTHREAD_SHARED_MUTEX 1
#include <pthread.h>

#if ENABLE_ASSERTS
#include <errno.h>
#endif
#else
#include <shared_mutex>
#endif

#include "UniqueLock.h"
#include "SharedLock.h"

namespace ngine::Threading
{
	struct SharedMutex
	{
		SharedMutex() noexcept = default;
		~SharedMutex() noexcept
		{
#if USE_PTHREAD_SHARED_MUTEX
			[[maybe_unused]] const int result = pthread_rwlock_destroy(&m_mutex);
			Assert(result == 0);
#endif
		}

		SharedMutex(const SharedMutex&) = delete;
		SharedMutex& operator=(const SharedMutex&) = delete;
		SharedMutex(SharedMutex&&) = delete;
		SharedMutex& operator=(SharedMutex&&) = delete;

		FORCE_INLINE bool LockExclusive() noexcept
		{
#if USE_WINDOWS_RW_MUTEX
			_Smtx_lock_exclusive(&m_mutex);
			return true;
#elif USE_PTHREAD_SHARED_MUTEX
			[[maybe_unused]] const int result = pthread_rwlock_wrlock(&m_mutex);
			Assert(result == 0);
			return result == 0;
#elif USE_SHARED_SPINLOCK
			// Start by acquiring the unique mutex, ensuring no one else is writing
			// Note: readers may still be present as they don't use this mutex
			const bool wasLocked = m_mutex.LockExclusive();
			Assert(wasLocked);
			if (LIKELY(wasLocked))
			{
				while (true)
				{
					int expectedState = m_state.Load();
					// Wait if writer is present
					while (expectedState != 0)
					{
						expectedState = m_state.Load();
					}

					if (m_state.CompareExchangeWeak(expectedState, -1))
					{
						return true;
					}
				}
				ExpectUnreachable();
			}
			else
			{
				return false;
			}
#else
			m_mutex.lock();
			return true;
#endif
		}
		[[nodiscard]] FORCE_INLINE bool TryLockExclusive() noexcept
		{
#if USE_WINDOWS_RW_MUTEX
			return _Smtx_try_lock_exclusive(&m_mutex) != 0;
#elif USE_PTHREAD_SHARED_MUTEX
			const int result = pthread_rwlock_trywrlock(&m_mutex);
			Assert(result != EINVAL);
			return result == 0;
#elif USE_SHARED_SPINLOCK
			if (m_mutex.TryLockExclusive())
			{
				int expectedState = m_state.Load();
				if (expectedState != 0)
				{
					m_mutex.UnlockExclusive();
					return false;
				}

				if (m_state.CompareExchangeStrong(expectedState, -1))
				{
					return true;
				}
				else
				{
					m_mutex.UnlockExclusive();
					return false;
				}
			}
			else
			{
				return false;
			}
#else
			return m_mutex.try_lock();
#endif
		}
		FORCE_INLINE bool UnlockExclusive() noexcept
		{
#if USE_WINDOWS_RW_MUTEX
			_Smtx_unlock_exclusive(&m_mutex);
			return true;
#elif USE_PTHREAD_SHARED_MUTEX
			[[maybe_unused]] const int result = pthread_rwlock_unlock(&m_mutex);
			Assert(result == 0);
			return result == 0;
#elif USE_SHARED_SPINLOCK
			m_mutex.UnlockExclusive();

			int expectedState = -1;
			Assert(expectedState == -1);
			const bool wasSet = m_state.CompareExchangeStrong(expectedState, 0);
			Assert(wasSet);
			return wasSet;
#else
			m_mutex.unlock();
			return true;
#endif
		}
		[[nodiscard]] FORCE_INLINE bool LockShared() noexcept
		{
#if USE_WINDOWS_RW_MUTEX
			_Smtx_lock_shared(&m_mutex);
			return true;
#elif USE_PTHREAD_SHARED_MUTEX
			[[maybe_unused]] const int result = pthread_rwlock_rdlock(&m_mutex);
			Assert(result == 0);
			return result == 0;
#elif USE_SHARED_SPINLOCK
			while (true)
			{
				int expectedState = m_state.Load();
				// Wait if writer is present
				while (expectedState == -1)
				{
					expectedState = m_state.Load();
				}

				if (m_state.CompareExchangeWeak(expectedState, expectedState + 1))
				{
					return true;
				}
			}
			ExpectUnreachable();
#else
			m_mutex.lock_shared();
			return true;
#endif
		}
		[[nodiscard]] FORCE_INLINE bool TryLockShared() noexcept
		{
#if USE_WINDOWS_RW_MUTEX
			return _Smtx_try_lock_shared(&m_mutex) != 0;
#elif USE_PTHREAD_SHARED_MUTEX
			[[maybe_unused]] const int result = pthread_rwlock_tryrdlock(&m_mutex);
			Assert(result != EINVAL && result != EAGAIN);
			return result == 0;
#elif USE_SHARED_SPINLOCK
			int expectedState = m_state.Load();
			if (expectedState == -1)
			{
				// Writer was present
				return false;
			}

			return m_state.CompareExchangeStrong(expectedState, expectedState + 1);
#else
			return m_mutex.try_lock_shared();
#endif
		}
		FORCE_INLINE bool UnlockShared() noexcept
		{
#if USE_WINDOWS_RW_MUTEX
			_Smtx_unlock_shared(&m_mutex);
			return true;
#elif USE_PTHREAD_SHARED_MUTEX
			const int result = pthread_rwlock_unlock(&m_mutex);
			Assert(result == 0);
			return result == 0;
#elif USE_SHARED_SPINLOCK
			[[maybe_unused]] const int previousValue = m_state.FetchSubtract(1);
			Assert(previousValue > 0);
			return previousValue > 0;
#else
			m_mutex.unlock_shared();
			return true;
#endif
		}
	private:
#if USE_WINDOWS_RW_MUTEX
		_Smtx_t m_mutex{nullptr};
#elif USE_PTHREAD_SHARED_MUTEX
		PUSH_CLANG_WARNINGS
		DISABLE_CLANG_WARNING("-Wunused-private-field")
		pthread_rwlock_t m_mutex PTHREAD_RWLOCK_INITIALIZER;
		POP_CLANG_WARNINGS
#elif USE_SHARED_SPINLOCK
		//! -1 = there is a unique writer
		//! 0 = no access to the mutex
		//! >= 1 number of readers
		Threading::Atomic<int> m_state{0};
		Threading::Mutex m_mutex;
#else
		std::shared_mutex m_mutex;
#endif
	};
}

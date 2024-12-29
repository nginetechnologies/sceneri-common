#pragma once

#include <Common/Assert/Assert.h>
#include <Common/Platform/CompilerWarnings.h>

#if PLATFORM_WINDOWS
#include <xthreads.h>

using _Mtx_t = struct _Mtx_internal_imp_t*;

extern "C"
{
#if (defined(_MSC_VER) && (_MSC_VER >= 1939))
	void __cdecl _Mtx_init_in_situ(_Mtx_t, int) noexcept;
	void __cdecl _Mtx_destroy_in_situ(_Mtx_t) noexcept;
	_Thrd_result __cdecl _Mtx_lock(_Mtx_t) noexcept;
	_Thrd_result __cdecl _Mtx_trylock(_Mtx_t) noexcept;
	_Thrd_result __cdecl _Mtx_unlock(_Mtx_t) noexcept;
#else
	void __cdecl _Mtx_init_in_situ(_Mtx_t, int);
	void __cdecl _Mtx_destroy_in_situ(_Mtx_t);
	_Thrd_result __cdecl _Mtx_lock(_Mtx_t);
	_Thrd_result __cdecl _Mtx_trylock(_Mtx_t);
	_Thrd_result __cdecl _Mtx_unlock(_Mtx_t);
#endif
}

#elif PLATFORM_APPLE
#define USE_PTHREAD_MUTEX 1

#if PLATFORM_APPLE
#include <sys/_pthread/_pthread_mutex_t.h>
#include <pthread_impl.h>

#define PTHREAD_MUTEX_INITIALIZER \
	{ \
		_PTHREAD_MUTEX_SIG_init, \
		{ \
			0 \
		} \
	}
#elif PLATFORM_EMSCRIPTEN
#define USE_EMSCRIPTEN_MUTEX 1
#include <emscripten/wasm_worker.h>
#else
#include <pthread.h>
#endif

#if ENABLE_ASSERTS
#include <errno.h>
#endif

extern "C"
{
	int pthread_mutex_destroy(pthread_mutex_t*);
	int pthread_mutex_lock(pthread_mutex_t*);
	int pthread_mutex_trylock(pthread_mutex_t*);
	int pthread_mutex_unlock(pthread_mutex_t*);
}

#elif PLATFORM_EMSCRIPTEN
#define USE_EMSCRIPTEN_MUTEX 1
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>

#elif PLATFORM_POSIX
#define USE_PTHREAD_MUTEX 1
#include <pthread.h>

#if ENABLE_ASSERTS
#include <errno.h>
#endif
#else
#include <mutex>
#endif

#include "UniqueLock.h"

namespace ngine::Threading
{
	struct ConditionVariable;

	struct Mutex
	{
		FORCE_INLINE Mutex() noexcept
		{
#if PLATFORM_WINDOWS
			_Mtx_init_in_situ(GetMutex(), 0x02);
#elif USE_EMSCRIPTEN_MUTEX
			emscripten_lock_init(&m_lock);
#endif
		}
		FORCE_INLINE ~Mutex() noexcept
		{
#if PLATFORM_WINDOWS
			_Mtx_destroy_in_situ(GetMutex());
#elif USE_PTHREAD_MUTEX
			[[maybe_unused]] const int result = pthread_mutex_destroy(&m_mutex);
			Assert(result == 0);
#endif
		}

		Mutex(const Mutex&) = delete;
		Mutex& operator=(const Mutex&) = delete;
		Mutex(Mutex&&) = delete;
		Mutex& operator=(Mutex&&) = delete;

		FORCE_INLINE bool LockExclusive() noexcept
		{
#if PLATFORM_WINDOWS
			[[maybe_unused]] const _Thrd_result result = _Mtx_lock(GetMutex());
			Assert(result == _Thrd_result::_Success);
			return result == _Thrd_result::_Success;
#elif USE_PTHREAD_MUTEX
			[[maybe_unused]] const int result = pthread_mutex_lock(&m_mutex);
			Assert(result == 0);
			return result == 0;
#elif USE_EMSCRIPTEN_MUTEX
			if (emscripten_is_main_browser_thread())
			{
				emscripten_lock_busyspin_waitinf_acquire(&m_lock);
			}
			else
			{
				emscripten_lock_waitinf_acquire(&m_lock);
			}
			return true;
#else
			m_mutex.lock();
			return true;
#endif
		}

		[[nodiscard]] FORCE_INLINE bool TryLockExclusive() noexcept
		{
#if PLATFORM_WINDOWS
			const _Thrd_result result = _Mtx_trylock(GetMutex());
			switch (result)
			{
				case _Thrd_result::_Success:
					return true;
				case _Thrd_result::_Busy:
					return false;
				default:
					ExpectUnreachable();
			}
#elif USE_PTHREAD_MUTEX
			const int result = pthread_mutex_trylock(&m_mutex);
			Assert(result != EINVAL && result != EAGAIN);
			return result == 0;
#elif USE_EMSCRIPTEN_MUTEX
			return emscripten_lock_try_acquire(&m_lock);
#else
			return m_mutex.try_lock();
#endif
		}

		FORCE_INLINE void UnlockExclusive() noexcept
		{
#if PLATFORM_WINDOWS
			_Mtx_unlock(GetMutex());
#elif USE_PTHREAD_MUTEX
			[[maybe_unused]] const int result = pthread_mutex_unlock(&m_mutex);
			Assert(result == 0);
#elif USE_EMSCRIPTEN_MUTEX
			emscripten_lock_release(&m_lock);
#else
			m_mutex.unlock();
#endif
		}
	private:
#if PLATFORM_WINDOWS
		[[nodiscard]] FORCE_INLINE _Mtx_t GetMutex() noexcept
		{
			return reinterpret_cast<_Mtx_t>(&m_storage);
		}
#endif
		friend ConditionVariable;

#if PLATFORM_WINDOWS
		alignas(8) ByteType m_storage[80];
#elif USE_PTHREAD_MUTEX
		PUSH_CLANG_WARNINGS
		DISABLE_CLANG_WARNING("-Wunused-private-field")
		pthread_mutex_t m_mutex PTHREAD_MUTEX_INITIALIZER;
		POP_CLANG_WARNINGS
#elif USE_EMSCRIPTEN_MUTEX
		emscripten_lock_t m_lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;
#else
		std::mutex m_mutex;
#endif
	};
}

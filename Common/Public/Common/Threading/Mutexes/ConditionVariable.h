#pragma once

#include "Mutex.h"
#if PLATFORM_WINDOWS
using _Cnd_t = struct _Cnd_internal_imp_t*;

extern "C"
{
#if (defined(_MSC_VER) && (_MSC_VER >= 1939))
	_CRTIMP2_PURE void __cdecl _Cnd_init_in_situ(_Cnd_t) noexcept;
	_CRTIMP2_PURE void __cdecl _Cnd_destroy_in_situ(_Cnd_t) noexcept;
	_CRTIMP2_PURE _Thrd_result __cdecl _Cnd_wait(_Cnd_t, _Mtx_t) noexcept; // TRANSITION, ABI: Always returns _Thrd_success
	_CRTIMP2_PURE _Thrd_result __cdecl _Cnd_broadcast(_Cnd_t) noexcept;    // TRANSITION, ABI: Always returns _Thrd_success
	_CRTIMP2_PURE _Thrd_result __cdecl _Cnd_signal(_Cnd_t) noexcept;       // TRANSITION, ABI: Always returns _Thrd_success
#else
	_CRTIMP2_PURE void __cdecl _Cnd_init_in_situ(_Cnd_t);
	_CRTIMP2_PURE void __cdecl _Cnd_destroy_in_situ(_Cnd_t);
	_CRTIMP2_PURE _Thrd_result __cdecl _Cnd_wait(_Cnd_t, _Mtx_t); // TRANSITION, ABI: Always returns _Thrd_success
	_CRTIMP2_PURE _Thrd_result __cdecl _Cnd_broadcast(_Cnd_t);    // TRANSITION, ABI: Always returns _Thrd_success
	_CRTIMP2_PURE _Thrd_result __cdecl _Cnd_signal(_Cnd_t);       // TRANSITION, ABI: Always returns _Thrd_success
#endif
}
#elif PLATFORM_APPLE

#include <sys/_pthread/_pthread_cond_t.h>

extern "C"
{
	int pthread_cond_destroy(pthread_cond_t*);
	int pthread_cond_signal(pthread_cond_t*);
	int pthread_cond_broadcast(pthread_cond_t*);
	int pthread_cond_wait(pthread_cond_t* __restrict, pthread_mutex_t* __restrict) __DARWIN_ALIAS_C(pthread_cond_wait);
}

#define PTHREAD_COND_INITIALIZER \
	{ \
		_PTHREAD_COND_SIG_init, \
		{ \
			0 \
		} \
	}

#elif PLATFORM_EMSCRIPTEN
static_assert(USE_EMSCRIPTEN_MUTEX);
#define USE_EMSCRIPTEN_ATOMIC_CONDITION_VARAIBLE 1
#include <emscripten/atomic.h>
#elif !USE_PTHREAD_MUTEX
#include <condition_variable>
#endif

namespace ngine::Threading
{
	struct ConditionVariable
	{
		FORCE_INLINE ConditionVariable() noexcept
		{
#if PLATFORM_WINDOWS
			_Cnd_init_in_situ(GetCondition());
#elif USE_EMSCRIPTEN_ATOMIC_CONDITION_VARAIBLE
			emscripten_condvar_init(&m_conditionVariable);
#endif
		}
		ConditionVariable(const ConditionVariable&) = delete;
		ConditionVariable& operator=(const ConditionVariable&) = delete;
		ConditionVariable(ConditionVariable&&) = delete;
		ConditionVariable& operator=(ConditionVariable&&) = delete;
		FORCE_INLINE ~ConditionVariable() noexcept
		{
#if PLATFORM_WINDOWS
			_Cnd_destroy_in_situ(GetCondition());
#elif USE_PTHREAD_MUTEX
			pthread_cond_destroy(&m_conditionVariable);
#endif
		}

		FORCE_INLINE void NotifyOne() noexcept
		{
#if PLATFORM_WINDOWS
			_Cnd_signal(GetCondition());
#elif USE_EMSCRIPTEN_ATOMIC_CONDITION_VARAIBLE
			emscripten_condvar_signal(&m_conditionVariable, 1);
#elif USE_PTHREAD_MUTEX
			pthread_cond_signal(&m_conditionVariable);
#else
			m_conditionVariable.notify_one();
#endif
		}

		FORCE_INLINE void NotifyAll() noexcept
		{
#if PLATFORM_WINDOWS
			_Cnd_broadcast(GetCondition());
#elif USE_EMSCRIPTEN_ATOMIC_CONDITION_VARAIBLE
			emscripten_condvar_signal(&m_conditionVariable, EMSCRIPTEN_NOTIFY_ALL_WAITERS);
#elif USE_PTHREAD_MUTEX
			pthread_cond_broadcast(&m_conditionVariable);
#else
			m_conditionVariable.notify_all();
#endif
		}

		FORCE_INLINE void Wait(UniqueLock<Mutex>& lock) noexcept
		{
#if PLATFORM_WINDOWS
			_Cnd_wait(GetCondition(), lock.GetMutex()->GetMutex());
#elif USE_EMSCRIPTEN_ATOMIC_CONDITION_VARAIBLE
			emscripten_condvar_waitinf(&m_conditionVariable, &lock.GetMutex()->m_lock);
#elif USE_PTHREAD_MUTEX
			pthread_cond_wait(&m_conditionVariable, &lock.GetMutex()->m_mutex);
#else
			std::mutex& mutex = lock.RelinquishLock()->m_mutex;
			std::unique_lock stdLock(mutex, std::adopt_lock);
			m_conditionVariable.wait(stdLock);
#endif
		}
	private:
#if PLATFORM_WINDOWS
		[[nodiscard]] FORCE_INLINE _Cnd_t GetCondition() noexcept
		{
			return reinterpret_cast<_Cnd_t>(&m_storage);
		}
#endif

#if PLATFORM_WINDOWS
		alignas(8) ByteType m_storage[72];
#elif USE_EMSCRIPTEN_ATOMIC_CONDITION_VARAIBLE
		emscripten_condvar_t m_conditionVariable = EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER;
#elif USE_PTHREAD_MUTEX
		pthread_cond_t m_conditionVariable = PTHREAD_COND_INITIALIZER;
#else
		std::condition_variable m_conditionVariable;
#endif
	};
}

#include "Threading/ThreadId.h"

#if PLATFORM_WINDOWS
#include <Platform/Windows.h>
#elif USE_PTHREAD
#include <pthread.h>
#elif USE_WEB_WORKERS
#include <emscripten/wasm_worker.h>
#endif

#include <thread>

namespace ngine::Threading
{
	[[nodiscard]] ThreadId ThreadId::GetCurrentThreadId()
	{
#if USE_PTHREAD
		static thread_local const ThreadId threadIdentifier = pthread_self();
		return threadIdentifier;
#elif USE_WEB_WORKERS
		static thread_local const ThreadId threadIdentifier = emscripten_wasm_worker_self_id();
		return threadIdentifier;
#else
		static thread_local const std::thread::id threadId = std::this_thread::get_id();
		return *reinterpret_cast<const ThreadId*>(&threadId);
#endif
	}

	ThreadId ThreadId::Get(const NativeHandleType handle)
	{
#if PLATFORM_WINDOWS
		return ThreadId(::GetThreadId(handle));
#elif USE_PTHREAD || USE_WEB_WORKERS
		return ThreadId(handle);
#endif
	}

#if PLATFORM_WINDOWS
	ThreadId::NativeHandleType ThreadId::GetHandle() const
	{
		return OpenThread(THREAD_ALL_ACCESS, false, m_value);
	}
#elif USE_PTHREAD
	ThreadId::NativeIdType ThreadId::GetId() const
	{
		return reinterpret_cast<const ThreadId::NativeIdType&>(m_value);
	}
#elif USE_WEB_WORKERS
	ThreadId::NativeIdType ThreadId::GetId() const
	{
		return (ThreadId::NativeIdType)m_value;
	}
#endif
}

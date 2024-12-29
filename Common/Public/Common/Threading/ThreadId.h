#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Platform/Pure.h>

#if PLATFORM_POSIX && SUPPORT_PTHREADS
#define USE_PTHREAD 1
#include <pthread.h>
#include <unistd.h>
#elif PLATFORM_WEB
#define USE_WEB_WORKERS 1
#endif

namespace ngine::Threading
{
	struct ThreadId
	{
#if PLATFORM_WINDOWS
		using NativeIdType = unsigned long;
		using NativeHandleType = void*;
		using StoredType = NativeIdType;
		inline static constexpr StoredType InvalidStoredType = Math::NumericLimits<StoredType>::Max;
#elif USE_PTHREAD
		using NativeIdType = pid_t;
		using NativeHandleType = pthread_t;
		using StoredType = NativeHandleType;
		inline static constexpr StoredType InvalidStoredType{0};
#elif USE_WEB_WORKERS
		using NativeIdType = uint32;
		using NativeHandleType = uint32;
		using StoredType = NativeHandleType;
		inline static constexpr StoredType InvalidStoredType{Math::NumericLimits<StoredType>::Max};
#endif

		ThreadId() = default;
		ThreadId(const StoredType value)
			: m_value(value)
		{
		}

		[[nodiscard]] PURE_STATICS static ThreadId GetCurrent()
		{
			static const thread_local ThreadId threadIdentifier = GetCurrentThreadId();
			return threadIdentifier;
		}
		[[nodiscard]] PURE_STATICS static ThreadId Get(const NativeHandleType handle);

		[[nodiscard]] bool IsValid() const
		{
			return m_value == InvalidStoredType;
		}

		[[nodiscard]] bool operator==(const ThreadId other)
		{
			return m_value == other.m_value;
		}
		[[nodiscard]] bool operator!=(const ThreadId other)
		{
			return m_value != other.m_value;
		}

#if PLATFORM_WINDOWS
		[[nodiscard]] PURE_STATICS NativeHandleType GetHandle() const;
#elif USE_PTHREAD || USE_WEB_WORKERS
		[[nodiscard]] NativeHandleType GetHandle() const
		{
			return m_value;
		}
#endif

#if PLATFORM_WINDOWS
		[[nodiscard]] NativeIdType GetId() const
		{
			return m_value;
		}
#elif USE_PTHREAD || USE_WEB_WORKERS
		[[nodiscard]] NativeIdType GetId() const;
#endif
	protected:
		[[nodiscard]] PURE_STATICS static ThreadId GetCurrentThreadId();
	protected:
		StoredType m_value{InvalidStoredType};
	};
}

#pragma once

#include <mutex>

namespace ngine::Threading
{
	struct RecursiveMutex
	{
		RecursiveMutex() noexcept = default;
		~RecursiveMutex() noexcept = default;

		RecursiveMutex(const RecursiveMutex&) = delete;
		RecursiveMutex& operator=(const RecursiveMutex&) = delete;
		RecursiveMutex(RecursiveMutex&&) = delete;
		RecursiveMutex& operator=(RecursiveMutex&&) = delete;

		FORCE_INLINE bool LockExclusive() noexcept
		{
			m_mutex.lock();
			return true;
		}

		[[nodiscard]] FORCE_INLINE bool TryLockExclusive() noexcept
		{
			return m_mutex.try_lock();
		}

		FORCE_INLINE void UnlockExclusive() noexcept
		{
			m_mutex.unlock();
		}
	private:
		std::recursive_mutex m_mutex;
	};
}

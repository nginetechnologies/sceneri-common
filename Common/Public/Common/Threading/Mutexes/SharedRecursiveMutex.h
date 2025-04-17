#pragma once

#include <Common/Threading/Mutexes/SharedMutex.h>
#include <Common/Memory/Containers/UnorderedMap.h>

namespace ngine::Threading
{
	struct SharedRecursiveMutex
	{
		SharedRecursiveMutex() noexcept = default;
		~SharedRecursiveMutex() noexcept = default;

		SharedRecursiveMutex(const SharedRecursiveMutex&) = delete;
		SharedRecursiveMutex& operator=(const SharedRecursiveMutex&) = delete;
		SharedRecursiveMutex(SharedRecursiveMutex&&) = delete;
		SharedRecursiveMutex& operator=(SharedRecursiveMutex&&) = delete;

		FORCE_INLINE bool LockExclusive() noexcept
		{
			TrackedMutex& __restrict trackedMutex = GetTrackedMutex();
			if (trackedMutex.readerCount == 0 && trackedMutex.writerCount == 0)
			{
				m_mutex.LockExclusive();
			}
			else if (trackedMutex.writerCount == 0 && trackedMutex.readerCount > 0)
			{
				m_mutex.UnlockShared();
				m_mutex.LockExclusive();
			}
			++trackedMutex.writerCount;

			return true;
		}

		[[nodiscard]] FORCE_INLINE bool TryLockExclusive() noexcept
		{
			TrackedMutex& __restrict trackedMutex = GetTrackedMutex();
			if (trackedMutex.writerCount > 0)
			{
				++trackedMutex.writerCount;
				return true;
			}
			if (trackedMutex.readerCount > 0)
			{
				// Same thread was previously reading, cannot lock exclusively
				return false;
			}
			const bool wasLocked = m_mutex.TryLockExclusive();
			if (wasLocked)
			{
				++trackedMutex.writerCount;
			}
			return wasLocked;
		}

		FORCE_INLINE void UnlockExclusive() noexcept
		{
			TrackedMutex& __restrict trackedMutex = GetTrackedMutex();
			Assert(trackedMutex.writerCount > 0);
			--trackedMutex.writerCount;
			if (trackedMutex.writerCount == 0)
			{
				m_mutex.UnlockExclusive();
				if (trackedMutex.readerCount > 0)
				{
					[[maybe_unused]] const bool wasLocked = m_mutex.LockShared();
					Assert(wasLocked);
				}
				else
				{
					RecursionTracker& tracker = GetTracker();
					tracker.Remove(tracker.Find(this));
				}
			}
		}

		FORCE_INLINE bool LockShared() noexcept
		{
			TrackedMutex& __restrict trackedMutex = GetTrackedMutex();
			if (trackedMutex.writerCount > 0)
			{
				++trackedMutex.writerCount;
			}
			else if (trackedMutex.readerCount > 0)
			{
				++trackedMutex.readerCount;
			}
			else if (trackedMutex.readerCount == 0)
			{
				const bool wasLocked = m_mutex.LockShared();
				if (wasLocked)
				{
					++trackedMutex.readerCount;
				}
				return wasLocked;
			}
			return true;
		}

		[[nodiscard]] FORCE_INLINE bool TryLockShared() noexcept
		{
			TrackedMutex& __restrict trackedMutex = GetTrackedMutex();
			if (trackedMutex.writerCount > 0 || trackedMutex.readerCount > 0)
			{
				LockShared();
				return true;
			}
			const bool wasLocked = m_mutex.TryLockShared();
			if (wasLocked)
			{
				++trackedMutex.readerCount;
			}
			return wasLocked;
		}

		FORCE_INLINE void UnlockShared() noexcept
		{
			TrackedMutex& __restrict trackedMutex = GetTrackedMutex();
			if (trackedMutex.writerCount > 0)
			{
				UnlockExclusive();
				return;
			}
			--trackedMutex.readerCount;
			if (trackedMutex.readerCount == 0)
			{
				m_mutex.UnlockShared();
				RecursionTracker& tracker = GetTracker();
				tracker.Remove(tracker.Find(this));
			}
		}
	private:
		struct TrackedMutex
		{
			uint32 readerCount{0};
			uint32 writerCount{0};
		};

		using RecursionTracker = UnorderedMap<SharedRecursiveMutex*, TrackedMutex>;

		[[nodiscard]] static RecursionTracker& GetTracker()
		{
			static thread_local RecursionTracker tracker;
			return tracker;
		}

		[[nodiscard]] TrackedMutex& GetTrackedMutex()
		{
			RecursionTracker& tracker = GetTracker();
			auto it = tracker.Find(this);
			if (it == tracker.end())
			{
				it = tracker.Emplace((SharedRecursiveMutex*)this, TrackedMutex{});
			}
			return it->second;
		}
	private:
		SharedMutex m_mutex;
	};
}

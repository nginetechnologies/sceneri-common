#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Assert/Assert.h>

#include "TryLock.h"
#include "AdoptLock.h"

namespace ngine::Threading
{
	template<typename MutexType>
	struct UniqueLock
	{
		UniqueLock() noexcept = default;
		FORCE_INLINE UniqueLock(MutexType& mutex) noexcept
			: m_pMutex(&mutex)
			, m_isOwned(m_pMutex->LockExclusive())
		{
		}
		FORCE_INLINE UniqueLock(TryLockType, MutexType& mutex) noexcept
			: m_pMutex(&mutex)
			, m_isOwned(mutex.TryLockExclusive())
		{
		}
		FORCE_INLINE UniqueLock(AdoptLockType, MutexType& mutex) noexcept
			: m_pMutex(&mutex)
			, m_isOwned(true)
		{
		}
		UniqueLock(const UniqueLock&) = delete;
		UniqueLock& operator=(const UniqueLock&) = delete;
		FORCE_INLINE UniqueLock(UniqueLock&& other) noexcept
			: m_pMutex(other.m_pMutex)
			, m_isOwned(other.m_isOwned)
		{
			other.m_pMutex = nullptr;
			other.m_isOwned = false;
		}
		FORCE_INLINE UniqueLock& operator=(UniqueLock&& other) noexcept
		{
			if (m_isOwned)
			{
				m_pMutex->UnlockExclusive();
			}
			m_pMutex = other.m_pMutex;
			m_isOwned = other.m_isOwned;
			other.m_pMutex = nullptr;
			other.m_isOwned = false;
			return *this;
		}
		FORCE_INLINE ~UniqueLock() noexcept
		{
			if (m_isOwned)
			{
				m_pMutex->UnlockExclusive();
			}
		}

		FORCE_INLINE void Lock() noexcept
		{
			Assert(m_pMutex != nullptr && !m_isOwned);
			m_isOwned = m_pMutex->LockExclusive();
		}
		FORCE_INLINE bool TryLock() noexcept
		{
			Assert(m_pMutex != nullptr && !m_isOwned);
			m_isOwned = m_pMutex->TryLockExclusive();
			return m_isOwned;
		}
		FORCE_INLINE void Unlock() noexcept
		{
			if (m_pMutex != nullptr && m_isOwned)
			{
				m_pMutex->UnlockExclusive();
				m_isOwned = false;
			}
		}

		[[nodiscard]] FORCE_INLINE bool IsLocked() const noexcept
		{
			return m_isOwned;
		}

		[[nodiscard]] MutexType* RelinquishLock() noexcept
		{
			MutexType* pMutex = m_pMutex;
			m_pMutex = nullptr;
			m_isOwned = false;
			return pMutex;
		}

		[[nodiscard]] FORCE_INLINE MutexType* GetMutex() const noexcept
		{
			return m_pMutex;
		}
	private:
		MutexType* m_pMutex = nullptr;
		bool m_isOwned = false;
	};
}

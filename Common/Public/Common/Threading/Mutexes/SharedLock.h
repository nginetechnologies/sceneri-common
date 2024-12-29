#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Assert/Assert.h>

#include "TryLock.h"
#include "AdoptLock.h"

namespace ngine::Threading
{
	template<typename MutexType>
	struct SharedLock
	{
		SharedLock() noexcept = default;
		FORCE_INLINE SharedLock(MutexType& mutex) noexcept
			: m_pMutex(&mutex)
			, m_isOwned(m_pMutex->LockShared())
		{
		}
		FORCE_INLINE SharedLock(TryLockType, MutexType& mutex) noexcept
			: m_pMutex(&mutex)
			, m_isOwned(mutex.TryLockShared())
		{
		}
		FORCE_INLINE SharedLock(AdoptLockType, MutexType& mutex) noexcept
			: m_pMutex(&mutex)
			, m_isOwned(true)
		{
		}
		SharedLock(const SharedLock&) = delete;
		SharedLock& operator=(const SharedLock&) = delete;
		FORCE_INLINE SharedLock(SharedLock&& other) noexcept
			: m_pMutex(other.m_pMutex)
			, m_isOwned(other.m_isOwned)
		{
			other.m_pMutex = nullptr;
			other.m_isOwned = false;
		}
		FORCE_INLINE SharedLock& operator=(SharedLock&& other) noexcept
		{
			if (m_isOwned)
			{
				m_pMutex->UnlockShared();
			}
			m_pMutex = other.m_pMutex;
			m_isOwned = other.m_isOwned;
			other.m_pMutex = nullptr;
			other.m_isOwned = false;
			return *this;
		}
		FORCE_INLINE ~SharedLock() noexcept
		{
			if (m_isOwned)
			{
				m_pMutex->UnlockShared();
			}
		}

		FORCE_INLINE void Lock() noexcept
		{
			Assert(m_pMutex != nullptr && !m_isOwned);
			m_isOwned = m_pMutex->LockShared();
		}
		[[nodiscard]] FORCE_INLINE bool TryLock() noexcept
		{
			Assert(m_pMutex != nullptr && !m_isOwned);
			m_isOwned = m_pMutex->TryLockShared();
			return m_isOwned;
		}
		FORCE_INLINE void Unlock() noexcept
		{
			if (m_pMutex != nullptr && m_isOwned)
			{
				m_pMutex->UnlockShared();
				m_isOwned = false;
			}
		}

		[[nodiscard]] FORCE_INLINE bool IsLocked() const noexcept
		{
			return m_isOwned;
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

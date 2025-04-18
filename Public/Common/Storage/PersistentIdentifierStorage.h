#pragma once

#include "Common/Platform/ForceInline.h"
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Storage/SaltedIdentifierStorage.h>
#include <Common/Storage/IdentifierArray.h>
#include <Common/Threading/Mutexes/SharedMutex.h>
namespace ngine
{
	template<typename IdentifierType_>
	struct PersistentIdentifierStorage
	{
		using IdentifierType = IdentifierType_;

		PersistentIdentifierStorage() = default;
		virtual ~PersistentIdentifierStorage() = default;

		[[nodiscard]] FORCE_INLINE IdentifierType Register(Guid guid)
		{
			Assert(guid.IsValid());
			Threading::UniqueLock lock(m_identifierLookupMapMutex);
			const IdentifierType identifier = m_identifierStorage.AcquireIdentifier();
			Assert(identifier.IsValid());
			if (LIKELY(identifier.IsValid()))
			{
				{
					Assert(!m_identifierLookupMap.Contains(guid));
					m_identifierLookupMap.Emplace(Guid(guid), IdentifierType(identifier));
				}
			}
			return identifier;
		}

		[[nodiscard]] FORCE_INLINE IdentifierType FindOrRegister(Guid guid)
		{
			Assert(guid.IsValid());
			{
				Threading::SharedLock lock(m_identifierLookupMapMutex);
				typename decltype(m_identifierLookupMap)::const_iterator it = m_identifierLookupMap.Find(guid);
				if (it != m_identifierLookupMap.end())
				{
					return it->second;
				}
			}

			Threading::UniqueLock lock(m_identifierLookupMapMutex);
			{
				typename decltype(m_identifierLookupMap)::const_iterator it = m_identifierLookupMap.Find(guid);
				if (it != m_identifierLookupMap.end())
				{
					return it->second;
				}
			}

			const IdentifierType identifier = m_identifierStorage.AcquireIdentifier();
			Assert(identifier.IsValid());
			if (LIKELY(identifier.IsValid()))
			{
				Assert(!m_identifierLookupMap.Contains(guid));
				m_identifierLookupMap.Emplace(Guid(guid), IdentifierType(identifier));
			}
			return identifier;
		}

		[[nodiscard]] FORCE_INLINE IdentifierType Find(Guid guid) const
		{
			Assert(guid.IsValid());
			{
				Threading::SharedLock lock(m_identifierLookupMapMutex);
				typename decltype(m_identifierLookupMap)::const_iterator it = m_identifierLookupMap.Find(guid);
				if (it != m_identifierLookupMap.end())
				{
					return it->second;
				}
			}

			Threading::UniqueLock lock(m_identifierLookupMapMutex);
			{
				typename decltype(m_identifierLookupMap)::const_iterator it = m_identifierLookupMap.Find(guid);
				if (it != m_identifierLookupMap.end())
				{
					return it->second;
				}
			}

			return {};
		}

		[[nodiscard]] FORCE_INLINE bool IsAvailable(Guid guid) const
		{
			Threading::SharedLock lock(m_identifierLookupMapMutex);
			return m_identifierLookupMap.Contains(guid);
		}

		[[nodiscard]] FORCE_INLINE bool IsAvailable(IdentifierType identifier) const
		{
			return m_identifierStorage.IsIdentifierActive(identifier);
		}

		template<typename ElementType>
		[[nodiscard]] FORCE_INLINE IdentifierArrayView<ElementType, IdentifierType>
		GetValidElementView(IdentifierArrayView<ElementType, IdentifierType> view) const
		{
			return m_identifierStorage.GetValidElementView(view);
		}

		template<typename ElementType>
		[[nodiscard]] FORCE_INLINE IdentifierArrayView<ElementType, IdentifierType>
		GetValidElementView(FixedIdentifierArrayView<ElementType, IdentifierType> view) const
		{
			return m_identifierStorage.GetValidElementView(view);
		}

		[[nodiscard]] typename IdentifierType::IndexType GetMaximumUsedIdentifierCount() const
		{
			return m_identifierStorage.GetMaximumUsedElementCount();
		}
	private:
		TSaltedIdentifierStorage<IdentifierType> m_identifierStorage;
		mutable Threading::SharedMutex m_identifierLookupMapMutex;
		UnorderedMap<Guid, IdentifierType, Guid::Hash> m_identifierLookupMap;
	};
}

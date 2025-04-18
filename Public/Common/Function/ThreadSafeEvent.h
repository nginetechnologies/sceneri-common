#pragma once

#include "Event.h"
#include "ForwardDeclarations/ThreadSafeEvent.h"
#include <Common/Threading/Mutexes/SharedMutex.h>

namespace ngine::ThreadSafe
{
	template<typename ReturnType, typename... ArgumentTypes, typename IdentifierType, size StorageSizeBytes, bool RequireUniqueIdentifiers>
	struct Event<ReturnType(IdentifierType, ArgumentTypes...), StorageSizeBytes, RequireUniqueIdentifiers>
		: protected ngine::Event<ReturnType(IdentifierType, ArgumentTypes...), StorageSizeBytes, RequireUniqueIdentifiers>
	{
		using BaseType = ngine::Event<ReturnType(IdentifierType, ArgumentTypes...), StorageSizeBytes, RequireUniqueIdentifiers>;
		using ListenerData = typename BaseType::ListenerData;
		using ListenerIdentifier = typename BaseType::ListenerIdentifier;

		using BaseType::BaseType;
		Event(const Event& other)
			: BaseType(static_cast<const BaseType&>(other))
		{
		}
		Event(Event&& other)
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		Event& operator=(const Event& other)
		{
			Threading::UniqueLock writeLock(m_mutex);
			Threading::SharedLock readLock(other.m_mutex);
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		Event& operator=(Event&& other)
		{
			Threading::UniqueLock writeLock(m_mutex);
			Threading::SharedLock readLock(other.m_mutex);
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}

		void Emplace(ListenerData&& listenerData)
		{
			Threading::UniqueLock writeLock(m_mutex);
			BaseType::Emplace(Forward<ListenerData>(listenerData));
		}

		template<typename ObjectType, typename Function>
		void Add(ObjectType& object, Function&& callback)
		{
			Assert(Memory::GetAddressOf(object) != nullptr);
			Threading::UniqueLock writeLock(m_mutex);
			BaseType::Add(object, Forward<Function>(callback));
		}

		template<typename ObjectType, typename Function>
		void Add(ObjectType* pObject, Function&& callback)
		{
			Expect(pObject != nullptr);
			Threading::UniqueLock writeLock(m_mutex);
			BaseType::Add(pObject, Forward<Function>(callback));
		}

		template<typename ObjectType>
		void Add(ObjectType& object, ReturnType (ObjectType::*callback)(ArgumentTypes...))
		{
			Assert(Memory::GetAddressOf(object) != nullptr);
			Threading::UniqueLock writeLock(m_mutex);
			BaseType::Add(object, callback);
		}

		template<typename ObjectType>
		void Add(ObjectType* pObject, ReturnType (ObjectType::*callback)(ArgumentTypes...))
		{
			Expect(pObject != nullptr);
			Threading::UniqueLock writeLock(m_mutex);
			BaseType::Add(pObject, callback);
		}

		template<typename ObjectType>
		void AddUnique(ObjectType& object, ReturnType (ObjectType::*callback)(ArgumentTypes...))
		{
			Assert(Memory::GetAddressOf(object) != nullptr);
			Threading::UniqueLock writeLock(m_mutex);
			if (!BaseType::Contains(&object))
			{
				BaseType::Add(object, callback);
			}
		}

		bool Remove(const IdentifierType identifier)
		{
			Threading::UniqueLock writeLock(m_mutex);
			return BaseType::Remove(identifier);
		}

		[[nodiscard]] Optional<ListenerData> Pop(const IdentifierType identifier)
		{
			Threading::UniqueLock writeLock(m_mutex);
			return BaseType::Pop(identifier);
		}

		template<typename ReturnType_ = ReturnType>
		EnableIf<TypeTraits::IsSame<ReturnType_, void>> operator()(ArgumentTypes... argumentTypes) const
		{
			Threading::SharedLock readLock(m_mutex);
			BaseType::operator()(argumentTypes...);
		}

		void operator()(ArgumentTypes... argumentTypes)
		{
			if constexpr (TypeTraits::IsSame<ReturnType, EventCallbackResult>)
			{
				Threading::UniqueLock writeLock(m_mutex);
				BaseType::operator()(argumentTypes...);
			}
			else
			{
				Threading::SharedLock readLock(m_mutex);
				BaseType::operator()(argumentTypes...);
			}
		}

		bool Execute(const IdentifierType identifier, ArgumentTypes... argumentTypes)
		{
			Threading::UniqueLock lock(m_mutex);
			return BaseType::Execute(identifier, argumentTypes...);
		}

		bool ExecuteAndRemove(const IdentifierType identifier, ArgumentTypes... argumentTypes)
		{
			Threading::UniqueLock lock(m_mutex);
			if (Optional<ListenerData> listenerData = BaseType::Pop(identifier))
			{
				lock.Unlock();
				listenerData->m_callback(listenerData->m_identifier, argumentTypes...);
				return true;
			}
			else
			{
				return false;
			}
		}

		void ExecuteAndClear(ArgumentTypes... argumentTypes)
		{
			decltype(BaseType::m_callbacks) callbacks;
			{
				Threading::UniqueLock lock(m_mutex);
				callbacks.MoveFrom(callbacks.begin(), BaseType::m_callbacks);
			}
			for (const ListenerData& __restrict callbackData : callbacks)
			{
				callbackData.m_callback(callbackData.m_identifier, argumentTypes...);
			}
		}

		void Clear()
		{
			Threading::UniqueLock writeLock(m_mutex);
			BaseType::Clear();
		}

		[[nodiscard]] bool HasCallbacks() const
		{
			Threading::SharedLock readLock(m_mutex);
			return BaseType::HasCallbacks();
		}

		[[nodiscard]] bool Contains(const IdentifierType identifier)
		{
			Threading::SharedLock readLock(m_mutex);
			return BaseType::Contains(identifier);
		}
	protected:
		// TODO: Replace this with a thread safe vector implementation
		mutable Threading::SharedMutex m_mutex;
	};
}

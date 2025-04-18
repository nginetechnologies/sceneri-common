#pragma once

#include "DynamicDelegate.h"

#include <Common/Memory/Containers/InlineVector.h>

namespace ngine::Scripting::VM
{
	struct DynamicEvent
	{
		using ListenerUserData = DynamicDelegate::UserData;

		DynamicEvent() = default;
		DynamicEvent(const DynamicEvent&) = delete;
		DynamicEvent& operator=(const DynamicEvent&) = delete;
		DynamicEvent(DynamicEvent&&) = default;
		DynamicEvent& operator=(DynamicEvent&&) = default;

		//! Adds an event listener
		DynamicDelegate& Emplace(DynamicDelegate&& delegate)
		{
			Assert(!Contains(delegate.m_userData));
			return m_delegates.EmplaceBack(Forward<DynamicDelegate>(delegate));
		}
		//! Adds an event listener, allowing duplicate entries
		DynamicDelegate& EmplaceWithDuplicates(DynamicDelegate&& delegate)
		{
			return m_delegates.EmplaceBack(Forward<DynamicDelegate>(delegate));
		}

		bool Remove(const ListenerUserData userData)
		{
			return m_delegates.RemoveFirstOccurrencePredicate(
				[userData](const DynamicDelegate& __restrict delegate) -> ErasePredicateResult
				{
					return delegate.IsBoundToObject(userData) ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
				}
			);
		}

		void RemoveAll(const ListenerUserData userData)
		{
			m_delegates.RemoveAllOccurrencesPredicate(
				[userData](const DynamicDelegate& __restrict delegate) -> ErasePredicateResult
				{
					return delegate.IsBoundToObject(userData) ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
				}
			);
		}

		template<typename IdentifierObjectType>
		bool Remove(const IdentifierObjectType& object)
		{
			if constexpr (TypeTraits::IsPrimitive<IdentifierObjectType>)
			{
				return m_delegates.RemoveFirstOccurrencePredicate(
					[object](const DynamicDelegate& __restrict delegate) -> ErasePredicateResult
					{
						return delegate.IsBoundToObject<IdentifierObjectType>(object) ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
					}
				);
			}
			else
			{
				return m_delegates.RemoveFirstOccurrencePredicate(
					[pObject = Memory::GetAddressOf(object)](const DynamicDelegate& __restrict delegate) -> ErasePredicateResult
					{
						return delegate.IsBoundToObject<IdentifierObjectType>(*pObject) ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
					}
				);
			}
		}

		template<typename IdentifierObjectType>
		void RemoveAll(const IdentifierObjectType& object)
		{
			if constexpr (TypeTraits::IsPrimitive<IdentifierObjectType>)
			{
				m_delegates.RemoveAllOccurrencesPredicate(
					[object](const DynamicDelegate& __restrict delegate) -> ErasePredicateResult
					{
						return delegate.IsBoundToObject<IdentifierObjectType>(object) ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
					}
				);
			}
			else
			{
				m_delegates.RemoveAllOccurrencesPredicate(
					[pObject = Memory::GetAddressOf(object)](const DynamicDelegate& __restrict delegate) -> ErasePredicateResult
					{
						return delegate.IsBoundToObject<IdentifierObjectType>(*pObject) ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
					}
				);
			}
		}

		[[nodiscard]] bool Contains(const ListenerUserData userData) const
		{
			return m_delegates.GetView().Any(
				[userData](const DynamicDelegate& __restrict delegate)
				{
					return delegate.IsBoundToObject(userData);
				}
			);
		}

		template<typename IdentifierObjectType>
		[[nodiscard]] bool Contains(const IdentifierObjectType& object) const
		{
			if constexpr (TypeTraits::IsPrimitive<IdentifierObjectType>)
			{
				return m_delegates.GetView().Any(
					[object](const DynamicDelegate& __restrict delegate)
					{
						return delegate.IsBoundToObject<IdentifierObjectType>(object);
					}
				);
			}
			else
			{
				return m_delegates.GetView().Any(
					[pObject = Memory::GetAddressOf(object)](const DynamicDelegate& __restrict delegate)
					{
						return delegate.IsBoundToObject<IdentifierObjectType>(*pObject);
					}
				);
			}
		}

		inline void Clear()
		{
			m_delegates.Clear();
		}

		template<typename... Args>
		void operator()(Args&&... args) const
		{
			Registers registers;
			static constexpr size UserDataArgumentOffset = 1;
			registers.PushArguments<UserDataArgumentOffset, Args...>(Forward<Args>(args)...);
			for (const DynamicDelegate& __restrict delegate : m_delegates)
			{
				delegate.m_callback(delegate.m_userData, registers[1], registers[2], registers[3], registers[4], registers[5]);
			}
			registers.PopArguments<UserDataArgumentOffset, Args...>();
		}

		void operator()(const Register R1, const Register R2, const Register R3, const Register R4, const Register R5) const
		{
			for (const DynamicDelegate& __restrict delegate : m_delegates)
			{
				delegate.m_callback(delegate.m_userData, R1, R2, R3, R4, R5);
			}
		}

		template<typename... Args>
		[[nodiscard]] bool BroadcastTo(const ListenerUserData userData, Args&&... args)
		{
			const OptionalIterator<DynamicDelegate> pDelegate = m_delegates.GetView().FindIf(
				[userData](const DynamicDelegate& __restrict delegate)
				{
					return delegate.IsBoundToObject(userData);
				}
			);
			if (pDelegate.IsValid())
			{
				(*pDelegate)(Forward<Args>(args)...);
				return true;
			}
			return false;
		}

		[[nodiscard]] bool BroadcastTo(
			const ListenerUserData userData, const Register R1, const Register R2, const Register R3, const Register R4, const Register R5
		)
		{
			const OptionalIterator<DynamicDelegate> pDelegate = m_delegates.GetView().FindIf(
				[userData](const DynamicDelegate& __restrict delegate)
				{
					return delegate.IsBoundToObject(userData);
				}
			);
			if (pDelegate.IsValid())
			{
				(*pDelegate)(R1, R2, R3, R4, R5);
				return true;
			}
			return false;
		}

		template<typename IdentifierObjectType, typename... Args>
		[[nodiscard]] bool BroadcastTo(const IdentifierObjectType& object, Args&&... args)
		{
			OptionalIterator<DynamicDelegate> pDelegate;
			if constexpr (TypeTraits::IsPrimitive<IdentifierObjectType>)
			{
				pDelegate = m_delegates.GetView().FindIf(
					[object](const DynamicDelegate& __restrict delegate)
					{
						return delegate.IsBoundToObject(object);
					}
				);
			}
			else
			{
				pDelegate = m_delegates.GetView().FindIf(
					[pObject = Memory::GetAddressOf(object)](const DynamicDelegate& __restrict delegate)
					{
						return delegate.IsBoundToObject(*pObject);
					}
				);
			}
			if (pDelegate.IsValid())
			{
				(*pDelegate)(Forward<Args>(args)...);
				return true;
			}
			return false;
		}

		[[nodiscard]] bool HasCallbacks() const
		{
			return m_delegates.HasElements();
		}
		[[nodiscard]] FORCE_INLINE static bool CompareRegisters(const Register a, const Register b)
		{
			return DynamicDelegate::CompareRegisters(a, b);
		}
	protected:
		InlineVector<DynamicDelegate, 2> m_delegates;
	};
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<Scripting::VM::DynamicEvent>
	{
		static constexpr auto Type =
			Reflection::Reflect<Scripting::VM::DynamicEvent>("75b4214a-654f-4b93-901b-81d7607b4eda"_guid, MAKE_UNICODE_LITERAL("Event"));
	};
}

#pragma once

#include <Common/Function/Function.h>
#include <Common/Memory/Containers/InlineVector.h>

#include "ForwardDeclarations/Event.h"
#include "EventCallbackResult.h"

namespace ngine
{
	template<typename ReturnType, typename... ArgumentTypes, typename IdentifierType, size StorageSizeBytes, bool RequireUniqueIdentifiers>
	struct Event<ReturnType(IdentifierType, ArgumentTypes...), StorageSizeBytes, RequireUniqueIdentifiers>
	{
	protected:
		using CallbackType = Function<ReturnType(IdentifierType, ArgumentTypes...), StorageSizeBytes>;
	public:
		using ListenerIdentifier = IdentifierType;
		struct ListenerData
		{
			ListenerData() = default;
			ListenerData(const ListenerData&) = delete;
			ListenerData& operator=(const ListenerData&) = delete;
			ListenerData(ListenerData&&) = default;
			ListenerData& operator=(ListenerData&&) = default;

			template<typename ObjectType, typename Function>
			ListenerData(ObjectType& object, Function&& callback)
				: m_identifier(&object)
				, m_callback(
						[callback = Forward<Function>(callback)](const IdentifierType identifier, ArgumentTypes... args) mutable
						{
							ObjectType& castObject = *reinterpret_cast<ObjectType*>(identifier);
							return callback(castObject, args...);
						}
					)
			{
			}

			template<typename ObjectType>
			ListenerData(ObjectType& object, ReturnType (ObjectType::*callback)(ArgumentTypes...))
				: m_identifier(&object)
				, m_callback(
						[callback](const IdentifierType identifier, ArgumentTypes... args)
						{
							ObjectType& castObject = *reinterpret_cast<ObjectType*>(identifier);
							return (castObject.*callback)(args...);
						}
					)
			{
			}

			template<typename Function>
			ListenerData(const ListenerIdentifier identifier, Function&& callback)
				: m_identifier(identifier)
				, m_callback(Forward<Function>(callback))
			{
			}

			template<typename ObjectType, typename Function>
			ListenerData(ObjectType* pObject, Function&& callback)
				: ListenerData(*pObject, Forward<Function>(callback))
			{
				Expect(pObject != nullptr);
			}

			template<typename ObjectType>
			ListenerData(ObjectType* pObject, ReturnType (ObjectType::*callback)(ArgumentTypes...))
				: ListenerData(*pObject, callback)
			{
				Expect(pObject != nullptr);
			}

			IdentifierType m_identifier;
			CallbackType m_callback;
		};

		ListenerData& Emplace(ListenerData&& listenerData)
		{
			if constexpr (RequireUniqueIdentifiers)
			{
				Assert(!Contains(listenerData.m_identifier));
			}
			return m_callbacks.EmplaceBack(Forward<ListenerData>(listenerData));
		}

		template<typename ObjectType, typename Function>
		void Add(ObjectType& object, Function&& callback)
		{
			if constexpr (RequireUniqueIdentifiers)
			{
				Assert(!Contains(&object));
			}
			m_callbacks.EmplaceBack(object, Forward<Function>(callback));
		}

		template<typename ObjectType, typename Function>
		void Add(ObjectType* pObject, Function&& callback)
		{
			Expect(pObject != nullptr);
			if constexpr (RequireUniqueIdentifiers)
			{
				Assert(!Contains(pObject));
			}
			m_callbacks.EmplaceBack(pObject, Forward<Function>(callback));
		}

		template<typename ObjectType>
		void Add(ObjectType& object, ReturnType (ObjectType::*callback)(ArgumentTypes...))
		{
			if constexpr (RequireUniqueIdentifiers)
			{
				Assert(!Contains(&object));
			}
			m_callbacks.EmplaceBack(object, callback);
		}

		template<typename ObjectType>
		void Add(ObjectType* pObject, ReturnType (ObjectType::*callback)(ArgumentTypes...))
		{
			Expect(pObject != nullptr);
			if constexpr (RequireUniqueIdentifiers)
			{
				Assert(!Contains(pObject));
			}
			m_callbacks.EmplaceBack(pObject, callback);
		}

		bool Remove(const IdentifierType identifier)
		{
			return m_callbacks.RemoveFirstOccurrencePredicate(
				[identifier](const ListenerData& __restrict callbackData) -> ErasePredicateResult
				{
					if (callbackData.m_identifier == identifier)
					{
						return ErasePredicateResult::Remove;
					}

					return ErasePredicateResult::Continue;
				}
			);
		}

		[[nodiscard]] Optional<ListenerData> Pop(const IdentifierType identifier)
		{
			auto it = m_callbacks.FindIf(
				[identifier](const ListenerData& __restrict callbackData)
				{
					return identifier == callbackData.m_identifier;
				}
			);
			if (it != m_callbacks.end())
			{
				ListenerData listenerData = Move(*it);
				m_callbacks.Remove(it);
				return Move(listenerData);
			}
			else
			{
				return {};
			}
		}

		[[nodiscard]] bool Contains(const IdentifierType identifier)
		{
			return m_callbacks.GetView().Any(
				[identifier](const ListenerData& __restrict callbackData)
				{
					return callbackData.m_identifier == identifier;
				}
			);
		}

		inline void Clear()
		{
			m_callbacks.Clear();
		}

		template<typename ReturnType_ = ReturnType>
		EnableIf<TypeTraits::IsSame<ReturnType_, void>> operator()(ArgumentTypes... argumentTypes) const
		{
			for (const ListenerData& __restrict callbackData : m_callbacks)
			{
				callbackData.m_callback(callbackData.m_identifier, argumentTypes...);
			}
		}

		void operator()(ArgumentTypes... argumentTypes)
		{
			if constexpr (TypeTraits::IsSame<ReturnType, void>)
			{
				for (const ListenerData& __restrict callbackData : m_callbacks)
				{
					callbackData.m_callback(callbackData.m_identifier, argumentTypes...);
				}
			}
			else
			{
				for (typename Callbacks::const_iterator it = m_callbacks.begin(), end = m_callbacks.end(); it != end;)
				{
					switch (it->m_callback(it->m_identifier, argumentTypes...))
					{
						case EventCallbackResult::Keep:
							++it;
							break;
						case EventCallbackResult::Remove:
							m_callbacks.Remove(it);
							--end;
							break;
					}
				}
			}
		}

		bool Execute(const IdentifierType identifier, ArgumentTypes... argumentTypes)
		{
			auto it = m_callbacks.FindIf(
				[identifier](const ListenerData& __restrict callbackData)
				{
					return identifier == callbackData.m_identifier;
				}
			);
			if (it != m_callbacks.end())
			{
				if constexpr (TypeTraits::IsSame<ReturnType, void>)
				{
					it->m_callback(identifier, argumentTypes...);
				}
				else
				{
					switch (it->m_callback(identifier, argumentTypes...))
					{
						case EventCallbackResult::Keep:
							break;
						case EventCallbackResult::Remove:
							m_callbacks.Remove(it);
							break;
					}
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		bool ExecuteAndRemove(const IdentifierType identifier, ArgumentTypes... argumentTypes)
		{
			auto it = m_callbacks.FindIf(
				[identifier](const ListenerData& __restrict callbackData)
				{
					return callbackData.m_identifier;
				}
			);
			if (it != m_callbacks.end())
			{
				ListenerData listenerData = Move(*it);
				m_callbacks.Remove(it);
				listenerData.m_callback(listenerData.m_identifier, argumentTypes...);
				return true;
			}
			else
			{
				return false;
			}
		}

		void ExecuteAndClear(ArgumentTypes... argumentTypes)
		{
			decltype(m_callbacks) callbacks = Move(m_callbacks);
			for (const ListenerData& __restrict callbackData : callbacks)
			{
				callbackData.m_callback(callbackData.m_identifier, argumentTypes...);
			}
		}

		[[nodiscard]] bool HasCallbacks() const
		{
			return m_callbacks.HasElements();
		}
	protected:
		using Callbacks = InlineVector<ListenerData, 2>;
		Callbacks m_callbacks;
	};
}

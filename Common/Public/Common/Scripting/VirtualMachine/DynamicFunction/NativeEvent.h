#pragma once

#include "DynamicEvent.h"

#include <Common/TypeTraits/IsInvocable.h>
#include <Common/Memory/Tuple.h>

namespace ngine::Scripting::VM
{
	template<typename SignatureType>
	struct NativeEvent;

	template<typename... ArgumentTypes_>
	struct NativeEvent<void(ArgumentTypes_...)> : private DynamicEvent
	{
		using BaseType = DynamicEvent;
		using SignatureType = void(ArgumentTypes_...);
		using ArgumentTypes = Tuple<ArgumentTypes_...>;

		template<auto Callback, typename ObjectType, typename = EnableIf<TypeTraits::IsInvocable<decltype(Callback), void, ArgumentTypes_...>>>
		void Add(ObjectType& object)
		{
			BaseType::Emplace(DynamicDelegate::Make<Callback, ObjectType>(object));
		}
		template<auto Callback, typename ObjectType, typename = EnableIf<TypeTraits::IsInvocable<decltype(Callback), void, ArgumentTypes_...>>>
		void AddWithDuplicates(ObjectType& object)
		{
			BaseType::EmplaceWithDuplicates(DynamicDelegate::Make<Callback, ObjectType>(object));
		}

		template<
			typename Identifier,
			typename CallbackObject,
			typename = EnableIf<
				TypeTraits::HasFunctionCallOperator<CallbackObject> &&
				TypeTraits::IsInvocable<decltype(CallbackObject::operator()), ArgumentTypes_...>>>
		void Add(const Identifier identifier, CallbackObject&& callback)
		{
			BaseType::Emplace(DynamicDelegate::Make<Identifier, CallbackObject>(identifier, Forward<CallbackObject>(callback)));
		}
		template<
			typename Identifier,
			typename CallbackObject,
			typename = EnableIf<
				TypeTraits::HasFunctionCallOperator<CallbackObject> &&
				TypeTraits::IsInvocable<decltype(CallbackObject::operator()), ArgumentTypes_...>>>
		void AddWithDuplicates(const Identifier identifier, CallbackObject&& callback)
		{
			BaseType::EmplaceWithDuplicates(DynamicDelegate::Make<Identifier, CallbackObject>(identifier, Forward<CallbackObject>(callback)));
		}

		using DynamicEvent::Remove;
		using DynamicEvent::RemoveAll;
		using DynamicEvent::Contains;
		using DynamicEvent::Clear;
		using DynamicEvent::HasCallbacks;

		void operator()(ArgumentTypes_&&... args) const
		{
			BaseType::operator()(Forward<ArgumentTypes_>(args)...);
		}

		[[nodiscard]] bool BroadcastTo(const ListenerUserData userData, ArgumentTypes_&&... args)
		{
			return BaseType::BroadcastTo(userData, Forward<ArgumentTypes_>(args)...);
		}

		template<typename IdentifierObjectType>
		[[nodiscard]] bool BroadcastTo(const IdentifierObjectType& object, ArgumentTypes_&&... args)
		{
			return BaseType::BroadcastTo(object, Forward<ArgumentTypes_>(args)...);
		}

		[[nodiscard]] DynamicEvent& GetDynamic()
		{
			return *this;
		}
		[[nodiscard]] const DynamicEvent& GetDynamic() const
		{
			return *this;
		}
	};
}

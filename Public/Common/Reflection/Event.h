#pragma once

#include "EventInfo.h"

#include <Common/Memory/Tuple.h>
#include <Common/TypeTraits/GetFunctionSignature.h>
#include <Common/TypeTraits/Select.h>
#include <Common/TypeTraits/MemberOwnerType.h>
#include <Common/TypeTraits/MemberType.h>
#include <Common/TypeTraits/IsFunctionPointer.h>
#include <Common/TypeTraits/IsMemberFunction.h>
#include <Common/TypeTraits/IsMemberPointer.h>
#include <Common/TypeTraits/ReturnType.h>
#include <Common/TypeTraits/Select.h>
#include <Common/Memory/CallMemberFunctionWithTuple.h>

namespace ngine::Reflection
{
	template<auto Event>
	struct ReflectedEvent;

	template<typename EventPointerType, typename... Arguments>
	struct Event : public EventInfo
	{
		using MemberOwnerType = TypeTraits::MemberOwnerType<EventPointerType>;
		using EventMemberType =
			TypeTraits::Select<TypeTraits::IsMemberPointer<EventPointerType>, TypeTraits::MemberType<EventPointerType>, EventPointerType>;
		using EventType = TypeTraits::Select<
			TypeTraits::IsFunctionPointer<EventPointerType>,
			TypeTraits::WithoutPointer<TypeTraits::ReturnType<EventMemberType>>,
			EventMemberType>;
	protected:
		template<typename Type>
		[[nodiscard]] inline static constexpr Reflection::Argument PopulateArgumentInfo(Reflection::Argument&& argumentInfo)
		{
			constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<Type>, Type>;
			using FilteredType = typename TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<Type>>, Type>;
			argumentInfo.m_type = Reflection::TypeDefinition::Get<FilteredType>();

#if COMPILER_CLANG
			// Workaround for Clang 11 bug, remove when Steam runtime is upgraded
			return Move(argumentInfo);
#else
			return argumentInfo;
#endif
		}

		template<size Index, typename ArgumentTypeTuple, typename ArgumentInfoTuple>
		inline static constexpr void EmplaceArgumentInfo(Array<Argument, sizeof...(Arguments)>& arguments, ArgumentInfoTuple&& remainingInfo)
		{
			arguments[Index] =
				PopulateArgumentInfo<typename ArgumentTypeTuple::template ElementType<Index>>(Move(remainingInfo.template Get<Index>()));

			if constexpr ((Index + 1) < ArgumentTypeTuple::ElementCount)
			{
				EmplaceArgumentInfo<Index + 1, ArgumentTypeTuple>(arguments, Forward<ArgumentInfoTuple>(remainingInfo));
			}
		}

		template<typename ArgumentTypeTuple, typename ArgumentInfoTuple>
		[[nodiscard]] inline static constexpr Array<Argument, sizeof...(Arguments)>
		PopulateArgumentInfos([[maybe_unused]] ArgumentInfoTuple&& argumentInfo)
		{
			static_assert(ArgumentTypeTuple::ElementCount == ArgumentInfoTuple::ElementCount);
			Array<Argument, sizeof...(Arguments)> arguments;
			if constexpr (sizeof...(Arguments) > 0)
			{
				EmplaceArgumentInfo<0, ArgumentTypeTuple>(arguments, Forward<ArgumentInfoTuple>(argumentInfo));
			}
			return arguments;
		}
	public:
		constexpr Event(const Guid guid, const ConstUnicodeStringView displayName, EventPointerType&& event, Arguments&&... arguments)
			: EventInfo(
					guid,
					displayName,
					[](const EventInfo& eventInfo) -> ArrayView<const Argument, uint8>
					{
						const Event& event = static_cast<const Event&>(eventInfo);
						return event.m_arguments.GetDynamicView();
					}
				)
			, m_event(Forward<EventPointerType>(event))
			, m_arguments(PopulateArgumentInfos<ArgumentTypes>(Tuple<Arguments...>{Forward<Arguments>(arguments)...}))
		{
		}

		[[nodiscard]] Guid GetGuid() const
		{
			return m_guid;
		}

		using SignatureType = typename EventType::SignatureType;
		using ArgumentTypes = typename EventType::ArgumentTypes;
		//! Whether the event member is a function, otherwise it should be a variable.
		inline static constexpr bool IsGetterFunction = TypeTraits::IsMemberFunction<EventPointerType> ||
		                                                TypeTraits::IsFunction<EventPointerType>;

		[[nodiscard]] constexpr EventPointerType GetEvent() const
		{
			return m_event;
		}
	private:
		const EventPointerType m_event;
		Array<Argument, sizeof...(Arguments)> m_arguments;
	};

	template<typename... EventTypes>
	struct Events : public Tuple<EventTypes...>
	{
		using BaseType = Tuple<EventTypes...>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};
	template<typename... Ts>
	Events(const Ts&...) -> Events<Ts...>;
	template<typename... Ts>
	Events(Ts&&...) -> Events<Ts...>;
};

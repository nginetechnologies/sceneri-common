#pragma once

#include "Tuple.h"

namespace ngine
{
	namespace Internal
	{
		template<typename ObjectType, typename Function, size Index, typename... ArgumentTypes>
		struct CallMemberFunctionWithTuple
		{
			template<typename... Vs>
			FORCE_INLINE static auto Apply(ObjectType& object, Function&& f, Tuple<ArgumentTypes...>&& t, Vs&&... args) noexcept -> decltype(auto)
			{
				using TupleType = Tuple<ArgumentTypes...>;
				using ElementType = typename TupleType::template ElementType<Index - 1>;
				return CallMemberFunctionWithTuple<ObjectType, Function, Index - 1, ArgumentTypes...>::Apply(
					object,
					Forward<Function>(f),
					Forward<TupleType>(t),
					static_cast<ElementType>(t.template Get<Index - 1>()),
					Forward<Vs>(args)...
				);
			}
		};

		template<typename ObjectType, typename Function, typename... ArgumentTypes>
		struct CallMemberFunctionWithTuple<ObjectType, Function, 0, ArgumentTypes...>
		{
			template<typename... Vs>
			FORCE_INLINE static auto Apply(ObjectType& object, Function&& f, Tuple<ArgumentTypes...>&&, Vs&&... args) noexcept -> decltype(auto)
			{
				return (object.*Forward<Function>(f))(Forward<Vs>(args)...);
			};
		};
	}

	template<typename ObjectType, typename Function, typename... ArgumentTypes>
	FORCE_INLINE auto CallMemberFunctionWithTuple(ObjectType& object, Function&& f, Tuple<ArgumentTypes...>&& t) noexcept -> decltype(auto)
	{
		return Internal::CallMemberFunctionWithTuple<ObjectType, Function, sizeof...(ArgumentTypes), ArgumentTypes...>::Apply(
			object,
			Forward<Function>(f),
			Forward<Tuple<ArgumentTypes...>>(t)
		);
	}
}

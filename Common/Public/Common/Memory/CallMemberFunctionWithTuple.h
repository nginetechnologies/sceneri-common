#pragma once

#include "Tuple.h"

namespace ngine
{
	namespace Internal
	{
		template<typename ObjectType, typename Function, typename TupleType, typename TupleType::SizeType... Is>
		FORCE_INLINE auto
		CallMemberFunctionWithTuple(ObjectType& object, Function&& f, TupleType&& t, TypeTraits::IntegerSequence<typename TupleType::SizeType, Is...>)
		{
			return (object.*Forward<Function>(f))(Move(t.template Get<Is>())...);
		}
	}

	template<typename ObjectType, typename Function, typename... ArgumentTypes>
	FORCE_INLINE auto CallMemberFunctionWithTuple(ObjectType& object, Function&& f, Tuple<ArgumentTypes...>&& t)
	{
		using TupleType = Tuple<ArgumentTypes...>;
		using IndexSequence = typename TupleType::IndexSequence;
		return Internal::CallMemberFunctionWithTuple<ObjectType, Function, TupleType>(
			object,
			Forward<Function>(f),
			Forward<Tuple<ArgumentTypes...>>(t),
			IndexSequence{}
		);
	}
}

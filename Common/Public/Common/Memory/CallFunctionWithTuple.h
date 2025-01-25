#pragma once

#include "Tuple.h"

namespace ngine
{
	namespace Internal
	{
		template<typename Function, typename TupleType, typename TupleType::SizeType... Is>
		FORCE_INLINE auto CallFunctionWithTuple(Function&& f, TupleType&& t, TypeTraits::IntegerSequence<typename TupleType::SizeType, Is...>)
		{
			return (Forward<Function>(f))(Move(t.template Get<Is>())...);
		}
	}

	template<typename Function, typename... ArgumentTypes>
	FORCE_INLINE auto CallFunctionWithTuple(Function&& f, Tuple<ArgumentTypes...>&& t)
	{
		using TupleType = Tuple<ArgumentTypes...>;
		using IndexSequence = typename TupleType::IndexSequence;
		return Internal::CallFunctionWithTuple<Function, TupleType>(Forward<Function>(f), Forward<Tuple<ArgumentTypes...>>(t), IndexSequence{});
	}
}

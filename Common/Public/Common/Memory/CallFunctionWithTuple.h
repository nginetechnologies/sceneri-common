#pragma once

#include "Tuple.h"

namespace ngine
{
	namespace Internal
	{
		template<typename Function, size Index, typename... ArgumentTypes>
		struct CallFunctionWithTuple
		{
			template<typename... Vs>
			FORCE_INLINE static auto Apply(Function&& f, Tuple<ArgumentTypes...>&& t, Vs&&... args) noexcept -> decltype(auto)
			{
				using TupleType = Tuple<ArgumentTypes...>;
				using ElementType = typename TupleType::template ElementType<Index - 1>;
				return CallFunctionWithTuple<Function, Index - 1, ArgumentTypes...>::Apply(
					Forward<Function>(f),
					Forward<TupleType>(t),
					static_cast<ElementType>(t.template Get<Index - 1>()),
					Forward<Vs>(args)...
				);
			}
		};

		template<typename Function, typename... ArgumentTypes>
		struct CallFunctionWithTuple<Function, 0, ArgumentTypes...>
		{
			template<typename... Vs>
			FORCE_INLINE static auto Apply(Function&& f, Tuple<ArgumentTypes...>&&, Vs&&... args) noexcept -> decltype(auto)
			{
				return Forward<Function>(f)(Forward<Vs>(args)...);
			};
		};
	}

	template<typename Function, typename... ArgumentTypes>
	FORCE_INLINE auto CallFunctionWithTuple(Function&& f, Tuple<ArgumentTypes...>&& t) noexcept -> decltype(auto)
	{
		return Internal::CallFunctionWithTuple<Function, sizeof...(ArgumentTypes), ArgumentTypes...>::Apply(
			Forward<Function>(f),
			Forward<Tuple<ArgumentTypes...>>(t)
		);
	}
}

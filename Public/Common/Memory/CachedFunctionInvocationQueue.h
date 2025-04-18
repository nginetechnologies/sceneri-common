#pragma once

#include "Tuple.h"
#include "CallFunctionWithTuple.h"

namespace ngine
{
	template<size ExtraSize, size MaximumQueuedInvokes, typename... ArgumentTypes>
	struct CachedFunctionInvocationQueue
	{
		template<typename Function>
		FORCE_INLINE CachedFunctionInvocationQueue(Function&& function) noexcept
			: m_function(Forward<Function>(function))
		{
		}

		template<typename Function>
		FORCE_INLINE void Bind(Function&& function) noexcept
		{
			m_function = Forward<Function>(function);
		}

		FORCE_INLINE void BindArguments(ArgumentTypes&&... arguments) noexcept
		{
			m_values.EmplaceBack(Forward<ArgumentTypes>(arguments)...);
		}

		FORCE_INLINE void operator()() noexcept
		{
			CallFunctionWithTuple(m_function, m_values.PopBack());
		}
	private:
		Function<void(ArgumentTypes...), ExtraSize> m_function;
		FlatVector<Tuple<ArgumentTypes...>, MaximumQueuedInvokes> m_values;
	};
}

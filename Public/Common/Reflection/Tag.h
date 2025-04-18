#pragma once

#include <Common/Memory/Tuple.h>
#include <Common/Tag/TagGuid.h>

namespace ngine::Reflection
{
	template<typename... ArgumentTypes>
	struct Tags : public Tuple<ArgumentTypes...>
	{
		using BaseType = Tuple<ArgumentTypes...>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};
	template<typename... Ts>
	Tags(const Ts&...) -> Tags<Ts...>;
};

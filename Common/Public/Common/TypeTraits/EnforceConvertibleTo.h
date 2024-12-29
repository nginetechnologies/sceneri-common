#pragma once

#include <Common/TypeTraits/IsConvertibleTo.h>
#include <Common/TypeTraits/All.h>

namespace ngine::TypeTraits
{
	template<class _First, class... _Rest>
	struct EnforceConvertibleTo
	{
		static_assert(TypeTraits::All<TypeTraits::IsConvertibleTo<_Rest, _First>...>, "All elements must be of the same type!");
		using Type = _First;
	};

	template<class _First>
	struct EnforceConvertibleTo<_First>
	{
		using Type = _First;
	};
}

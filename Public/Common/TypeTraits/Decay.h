#pragma once

#include "WithoutReference.h"
#include "WithoutConstOrVolatile.h"

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename Type_>
		struct Decay
		{
			using TypeWithoutReference = WithoutReference<Type_>;
			using TypeWithoutConstOrVolatile = WithoutConstOrVolatile<TypeWithoutReference>;
			using Type = TypeWithoutConstOrVolatile;
		};
	}

	template<typename Type>
	using Decay = typename Internal::Decay<Type>::Type;
}

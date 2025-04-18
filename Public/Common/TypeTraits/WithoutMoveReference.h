#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<class TypeIn>
		struct WithoutMoveReference
		{
			using Type = TypeIn;
		};

		template<class TypeIn>
		struct WithoutMoveReference<TypeIn&&>
		{
			using Type = TypeIn;
		};
	}

	template<class Type>
	using WithoutMoveReference = typename Internal::WithoutMoveReference<Type>::Type;
}

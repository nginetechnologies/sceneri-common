#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<class InType>
		struct WithConst
		{
			using Type = const InType;
		};

		template<class InType>
		struct WithConst<const InType>
		{
			using Type = const InType;
		};
	}

	template<class Type>
	using WithConst = typename Internal::WithConst<Type>::Type;
}

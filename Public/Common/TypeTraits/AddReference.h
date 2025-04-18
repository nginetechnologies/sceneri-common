#pragma once

#include <Common/TypeTraits/Void.h>

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<class Type, class = void>
		struct AddReference
		{
			using LValueType = Type;
			using RValueType = Type;
		};

		template<class Type>
		struct AddReference<Type, Void<Type&>>
		{
			using LValueType = Type&;
			using RValueType = Type&&;
		};
	}

	template<class Type>
	using AddLValueReference = typename Internal::AddReference<Type>::LValueType;

	template<class Type>
	using AddRValueReference = typename Internal::AddReference<Type>::RValueType;
}

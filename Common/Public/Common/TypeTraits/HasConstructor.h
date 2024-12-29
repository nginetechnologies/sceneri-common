#pragma once

#include <Common/TypeTraits/DeclareValue.h>

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T, typename... Args>
		class HasConstructor
		{
			typedef char yes;
			typedef struct
			{
				char arr[2];
			} no;

			template<typename U>
			static decltype(U(TypeTraits::DeclareValue<Args>()...), yes()) test(int);

			template<typename>
			static no test(...);
		public:
			static const bool Value = sizeof(test<T>(0)) == sizeof(yes);
		};
	}

	template<typename Type, typename... Args>
	inline static constexpr bool HasConstructor = Internal::HasConstructor<Type, Args...>::Value;
}

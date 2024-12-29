#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T, typename... Arguments>
		struct IsTriviallyConstructible
		{
			inline static constexpr bool Value = __is_trivially_constructible(T, Arguments...);
		};
	}

	template<typename Type, typename... Arguments>
	inline static constexpr bool IsTriviallyConstructible = Internal::IsTriviallyConstructible<Type, Arguments...>::Value;
	template<typename Type>
	inline static constexpr bool IsTriviallyCopyConstructible = IsTriviallyConstructible<Type, const Type&>;
	template<typename Type>
	inline static constexpr bool IsTriviallyMoveConstructible = IsTriviallyConstructible<Type, Type&&>;
}

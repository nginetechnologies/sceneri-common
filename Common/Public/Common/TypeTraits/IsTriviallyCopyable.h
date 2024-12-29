#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T>
		struct IsTriviallyCopyable
		{
			inline static constexpr bool Value = __is_trivially_copyable(T);
		};
	}

	template<typename Type>
	inline static constexpr bool IsTriviallyCopyable = Internal::IsTriviallyCopyable<Type>::Value;
}

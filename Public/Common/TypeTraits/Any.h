#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<bool... Values>
		struct Any
		{
			inline static constexpr bool Value = (Values | ...) != 0;
		};

		template<bool Value_>
		struct Any<Value_>
		{
			inline static constexpr bool Value = Value_;
		};

		template<>
		struct Any<>
		{
			inline static constexpr bool Value = false;
		};
	}

	template<bool... Values>
	inline static constexpr bool Any = Internal::Any<Values...>::Value;
}

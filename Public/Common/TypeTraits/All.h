#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<bool... Values>
		struct All
		{
			inline static constexpr bool Value = (Values & ...) != 0;
		};

		template<bool Value_>
		struct All<Value_>
		{
			inline static constexpr bool Value = Value_;
		};

		template<>
		struct All<>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<bool... Values>
	inline static constexpr bool All = Internal::All<Values...>::Value;
}

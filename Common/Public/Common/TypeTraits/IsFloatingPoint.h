#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename Type>
		struct IsFloatingPoint
		{
			inline static constexpr bool Value = false;
		};

		template<>
		struct IsFloatingPoint<float>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsFloatingPoint<double>
		{
			inline static constexpr bool Value = true;
		};
		template<>
		struct IsFloatingPoint<long double>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool IsFloatingPoint = Internal::IsFloatingPoint<Type>::Value;
}
